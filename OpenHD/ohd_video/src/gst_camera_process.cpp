#include "gst_camera_process.h"

#include <poll.h>
#include <sys/prctl.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include "config_paths.h"
#ifdef OPENHD_GSTREAMER_PRESENT
#include "gstreamerstream.h"
#endif
#ifdef OPENHD_ROCKCHIP_MPP_PRESENT
#include "rockchip_mpp_stream.h"
#endif
#include "libcamera_app_stream.h"
#include "openhd_config.h"
#include "video_worker_child.h"
#include "video_worker_frame.h"

namespace {
using namespace openhd::video_worker;
using Clock = std::chrono::steady_clock;
constexpr auto kWatchdogTimeout = std::chrono::seconds(30);
volatile std::sig_atomic_t worker_stop_requested = 0;
void stop_worker(int) { worker_stop_requested = 1; }
struct Status {
  openhd::LinkActionHandler::CamInfo camera;
  int64_t progress_ms;
  int recording;
};

bool external_plugin() {
  return openhd::PluginManager::instance().is_plugin_loaded("topotek") ||
         openhd::PluginManager::instance().is_plugin_loaded("siyi");
}

}  // namespace

CameraProcess::CameraProcess(std::shared_ptr<CameraHolder> camera,
                             openhd::ON_ENCODE_FRAME_CB output,
                             bool video_outputs, Backend backend)
    : CameraStream(std::move(camera), std::move(output)),
      m_video_outputs(video_outputs),
      m_backend(backend) {
  m_armed = openhd::ArmingStateHelper::instance().is_currently_armed();
  refresh_settings(true);
  m_camera_holder->register_listener([this]() { refresh_settings(true); });
  m_camera_holder->register_video_bitrate_listener([this](int bitrate) {
    const auto info = openhd::LinkActionHandler::instance().get_cam_info(
        m_camera_holder->get_camera().index);
    handle_change_bitrate_request({bitrate, false});
    if (m_backend == Backend::GStreamer && !info.supports_variable_bitrate)
      refresh_settings(true);
  });
  m_camera_holder->register_video_qp_listener([this](int minimum, int maximum) {
    std::lock_guard<std::mutex> lock(m_control_mutex);
    m_qp_min = minimum;
    m_qp_max = maximum;
    m_snapshot = m_camera_holder->settings_snapshot();
    if (m_backend != Backend::Mpp) ++m_revision;
  });
  if (m_backend == Backend::Mpp) {
    m_camera_holder->register_video_orientation_listener([this](int degree, int flip) {
      std::lock_guard<std::mutex> lock(m_control_mutex);
      m_rotation_degree = degree;
      m_flip = flip;
      m_snapshot = m_camera_holder->settings_snapshot();
    });
    m_camera_holder->register_video_roi_listener(
        [this]() { refresh_settings(true); });
    m_camera_holder->register_video_recording_listener(
        [this]() { refresh_settings(true); });
    m_camera_holder->register_video_debug_listener(
        [this]() { refresh_settings(true); });
    m_camera_holder->register_video_force_keyframe_listener(
        [this]() { ++m_keyframe_revision; });
  }
}

void CameraProcess::refresh_settings(bool restart) {
  std::lock_guard<std::mutex> lock(m_control_mutex);
  m_snapshot = m_camera_holder->settings_snapshot();
  m_rotation_degree = m_camera_holder->get_settings().camera_rotation_degree;
  m_flip = m_camera_holder->get_settings().openhd_flip;
  m_settings_restart = m_settings_restart || restart;
  ++m_revision;
}

CameraProcess::~CameraProcess() {
  m_camera_holder->register_listener(nullptr);
  m_camera_holder->register_video_bitrate_listener(nullptr);
  m_camera_holder->register_video_qp_listener(nullptr);
  if (m_backend == Backend::Mpp) {
    m_camera_holder->register_video_orientation_listener(nullptr);
    m_camera_holder->register_video_roi_listener(nullptr);
    m_camera_holder->register_video_recording_listener(nullptr);
    m_camera_holder->register_video_debug_listener(nullptr);
    m_camera_holder->register_video_force_keyframe_listener(nullptr);
  }
  terminate_looping();
}

void CameraProcess::start_looping() {
  if (m_running.exchange(true)) return;
  m_thread = std::thread(&CameraProcess::run, this);
}

void CameraProcess::terminate_looping() {
  m_running = false;
  if (m_thread.joinable()) m_thread.join();
}

void CameraProcess::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  // Keep persistence and plugin commands in the parent. The worker's existing
  // bitrate handler still performs all platform/encoder-specific clamping.
  std::lock_guard<std::mutex> lock(m_control_mutex);
  if (m_backend == Backend::GStreamer &&
      m_camera_holder->get_camera().requires_rockchip_rv_pipeline())
    return;
  int requested = lb.recommended_encoder_bitrate_kbits;
  if (lb.is_link_capacity_limit)
    requested =
        std::min(requested, m_camera_holder->get_settings().h26x_bitrate_kbits);
  if (m_backend == Backend::GStreamer) {
#ifdef OPENHD_GSTREAMER_PRESENT
    requested =
        GStreamerStream::clamp_runtime_bitrate(*m_camera_holder, requested);
#endif
  } else {
    requested = m_camera_holder->clamp_video_bitrate_kbits(requested);
    if (m_backend == Backend::Libcamera) requested = std::max(2000, requested);
  }
  const bool changed = m_bitrate != requested;
  m_bitrate = requested;
  m_capacity_limit = lb.is_link_capacity_limit;
  if (!lb.is_link_capacity_limit &&
      m_camera_holder->get_settings().h26x_bitrate_kbits != requested) {
    m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits = requested;
    m_camera_holder->persist(false);
  }
  if (!lb.is_link_capacity_limit) {
    m_snapshot = m_camera_holder->settings_snapshot();
    if (m_backend == Backend::GStreamer) ++m_revision;
  }
  if (changed &&
      m_camera_holder->get_camera().camera_type == X_CAM_TYPE_EXTERNAL_IP)
    m_camera_holder->notify_plugin_video_bitrate_changed(requested);
}

void CameraProcess::handle_update_arming_state(bool armed) {
  std::lock_guard<std::mutex> lock(m_control_mutex);
  m_armed = armed;
}

void CameraProcess::run() {
  const auto camera = m_camera_holder->get_camera();
  auto console = openhd::log::create_or_get(
      "cam" + std::to_string(camera.index) + "-supervisor");
  while (m_running) {
    Child child;
    auto& actions = openhd::LinkActionHandler::instance();
    actions.set_cam_info_status(camera.index, CAM_STATUS_RESTARTING);
    actions.set_cam_info_perf(camera.index, 0, 0);
    actions.set_cam_info_recording_active(camera.index, false);
    std::vector<std::string> args{
        m_backend == Backend::Mpp         ? "--mpp-camera-worker"
        : m_backend == Backend::Libcamera ? "--libcamera-worker"
                                          : "--gst-camera-worker",
        std::to_string(camera.camera_type),
        std::to_string(camera.index),
        std::to_string(is_usb_camera(camera.camera_type)
                           ? camera.usb_v4l2_device_number
                           : 0),
        m_video_outputs ? "1" : "0",
        external_plugin() ? "1" : "0",
        getConfigBasePath(),
        getVideoPath(),
        openhd::is_wifi_monitor_card_emulate_override_enabled() ? "1" : "0",
        std::to_string(getpid())};
    // Execute this process's inode, including while an update has replaced
    // its installed path. Parent and worker must use the same private ABI.
    if (!child.start("/proc/self/exe", std::move(args))) {
      console->error("Cannot start camera worker");
      for (int i = 0; i < 20 && m_running; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    console->info("Started camera worker pid {}", child.pid);
    auto last_progress = Clock::now();
    int64_t progress_ms = 0;
    unsigned sent_revision = m_revision.load() - 1;
    FrameAssembler assembler;
    bool failed = false;
    while (m_running && !child.reap()) {
      Control control;
      {
        std::lock_guard<std::mutex> lock(m_control_mutex);
        const auto revision = m_revision.load();
        if (sent_revision != revision &&
            send_packet(
                child.fd,
                {Type::Settings, revision, m_settings_restart ? 1u : 0u},
                m_snapshot.data(), m_snapshot.size())) {
          sent_revision = revision;
          m_settings_restart = false;
        }
        control = {m_bitrate, m_qp_min,         m_qp_max,
                   m_armed,   m_capacity_limit, m_keyframe_revision.load(),
                   m_rotation_degree, m_flip};
      }
      send_packet(child.fd, {Type::Control}, &control, sizeof(control));
      pollfd event{child.fd, POLLIN, 0};
      poll(&event, 1, 50);
      // Bound each drain so a busy camera cannot starve shutdown/the watchdog.
      for (int i = 0; i < 256; ++i) {
        Header header;
        std::vector<uint8_t> payload;
        const auto received = receive_packet(child.fd, header, payload);
        if (received <= 0) {
          failed = received < 0;
          break;
        }
        if (header.type == Type::Status && payload.size() == sizeof(Status)) {
          Status status;
          std::memcpy(&status, payload.data(), sizeof(status));
          actions.set_cam_info(camera.index, status.camera);
          if (status.progress_ms != progress_ms) {
            progress_ms = status.progress_ms;
            last_progress = Clock::now();
          }
          // Recording can be disabled by the worker's free-space check. Only
          // accept state from the latest settings snapshot, never an old ACK.
          if (header.sequence == m_revision.load() &&
              status.recording !=
                  m_camera_holder->get_settings().air_recording) {
            m_camera_holder->unsafe_get_settings().air_recording =
                status.recording;
            m_camera_holder->persist(false);
            refresh_settings(false);
          }
        } else if (header.type == Type::Log &&
                   payload.size() == sizeof(openhd::log::MavlinkLogMessage)) {
          openhd::log::MavlinkLogMessage message;
          std::memcpy(&message, payload.data(), sizeof(message));
          message.message[sizeof(message.message) - 1] = 0;
          openhd::log::MavlinkLogMessageBuffer::instance().enqueue_log_message(
              message);
        } else if (assembler.accept(header, payload)) {
          last_progress = Clock::now();
          try {
            if (m_output_cb) m_output_cb(camera.index, assembler.frame);
          } catch (const std::exception& error) {
            console->warn("Video forwarding failed: {}", error.what());
          } catch (...) {
            console->warn("Video forwarding failed with an unknown exception");
          }
        }
      }
      if (failed || Clock::now() - last_progress > kWatchdogTimeout) {
        console->error(
            "Camera worker disconnected or stalled; restarting "
            "camera");
        child.kill();
        failed = true;
        break;
      }
    }
    if (m_running && !failed)
      console->warn("Camera worker exited (wait status {}); restarting camera",
                    child.exit_status);
    if (!m_running && !failed)
      send_packet(child.fd, {Type::Stop});
    else
      child.kill();
    const auto graceful_deadline = Clock::now() + std::chrono::seconds(6);
    while (!child.reap()) {
      if (Clock::now() >= graceful_deadline) child.kill();
      if (!m_running &&
          Clock::now() >= graceful_deadline + std::chrono::seconds(1))
        break;
      // Never start another worker until the previous one has been reaped.
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    actions.set_cam_info_status(camera.index, CAM_STATUS_RESTARTING);
    actions.set_cam_info_perf(camera.index, 0, 0);
    actions.set_cam_info_recording_active(camera.index, false);
    for (int i = 0; i < 10 && m_running; ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int run_camera_worker_if_requested(int argc, char* argv[]) {
  if (argc < 2) return -1;
  const bool use_mpp = std::strcmp(argv[1], "--mpp-camera-worker") == 0;
  const bool use_libcamera = std::strcmp(argv[1], "--libcamera-worker") == 0;
  if (!use_mpp && !use_libcamera &&
      std::strcmp(argv[1], "--gst-camera-worker") != 0)
    return -1;
  if (argc != 11) return EXIT_FAILURE;
  // A parent's death must not leave a camera process holding the device open.
  prctl(PR_SET_PDEATHSIG, SIGKILL);
  if (getppid() != std::atoi(argv[10])) return EXIT_FAILURE;
  std::signal(SIGTERM, stop_worker);
  std::signal(SIGINT, stop_worker);
  // Native libcamera execs a nested encoder. It must not retain the private
  // parent/worker socket or keep the parent from observing worker death.
  fcntl(3, F_SETFD, FD_CLOEXEC);
  int socket_type = 0;
  socklen_t socket_type_size = sizeof(socket_type);
  if (getsockopt(3, SOL_SOCKET, SO_TYPE, &socket_type, &socket_type_size) !=
          0 ||
      socket_type != SOCK_SEQPACKET)
    return EXIT_FAILURE;
  int capacity = 4 * 1024 * 1024;
  setsockopt(3, SOL_SOCKET, SO_SNDBUF, &capacity, sizeof(capacity));
  setConfigBasePath(argv[7]);
  setVideoPath(argv[8]);
  openhd::set_wifi_monitor_card_emulate_override(std::atoi(argv[9]) != 0);
  XCamera camera{};
  camera.camera_type = std::stoi(argv[2]);
  camera.index = std::stoi(argv[3]);
  camera.usb_v4l2_device_number = std::stoi(argv[4]);
  if (camera.index < 0 || camera.index > 1) return EXIT_FAILURE;
  auto holder = std::make_shared<CameraHolder>(camera, true);
  uint32_t frame_sequence = 0;
  std::atomic<int64_t> last_frame_ms{0};
  openhd::ON_ENCODE_FRAME_CB output =
      [&](int, const openhd::FragmentedVideoFrame& frame) {
        last_frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            Clock::now().time_since_epoch())
                            .count();
        send_frame(3, ++frame_sequence, frame);
      };
  std::unique_ptr<CameraStream> stream;
#ifdef OPENHD_GSTREAMER_PRESENT
  GStreamerStream* gst = nullptr;
#endif
  auto create_stream = [&]() {
    if (use_mpp) {
#ifdef OPENHD_ROCKCHIP_MPP_PRESENT
      stream = std::make_unique<RockchipMppStream>(holder, output);
#else
      std::_Exit(EXIT_FAILURE);
#endif
    } else if (use_libcamera) {
      stream = std::make_unique<LibcameraAppStream>(holder, output);
    } else {
#ifdef OPENHD_GSTREAMER_PRESENT
      auto instance = std::make_unique<GStreamerStream>(
          holder, output, std::atoi(argv[5]) != 0, std::atoi(argv[6]) != 0);
      gst = instance.get();
      stream = std::move(instance);
#else
      std::_Exit(EXIT_FAILURE);
#endif
    }
  };
  // Native settings are applied with the previous backend stopped. If stop or
  // destruction hangs, the parent watchdog kills this worker, never OpenHD.
  unsigned native_revision = 0;
  Control applied;
  bool native_enabled = true;
  bool started = false;
  auto last_status = Clock::now() - std::chrono::seconds(1);
  while (true) {
    pollfd event{3, POLLIN, 0};
    poll(&event, 1, 50);
    Header header;
    std::vector<uint8_t> payload;
    const int received = receive_packet(3, header, payload);
    if (received < 0) std::_Exit(EXIT_SUCCESS);
    if (worker_stop_requested || (received > 0 && header.type == Type::Stop)) {
      if (stream) stream->terminate_looping();
      std::_Exit(EXIT_SUCCESS);
    }
    if (received > 0 && header.type == Type::Settings) {
      const std::string snapshot(payload.begin(), payload.end());
      if (use_mpp || use_libcamera) {
        stream.reset();
        if (!holder->apply_settings_snapshot(snapshot))
          std::_Exit(EXIT_FAILURE);
        native_revision = header.sequence;
        native_enabled = holder->get_settings().enable_streaming;
        create_stream();
        stream->handle_update_arming_state(applied.armed);
        if (applied.bitrate >= 0)
          stream->handle_change_bitrate_request({applied.bitrate, false});
#ifdef OPENHD_ROCKCHIP_MPP_PRESENT
        if (use_mpp && applied.qp_min >= 0 && applied.qp_max >= 0)
          static_cast<RockchipMppStream*>(stream.get())
              ->set_qp_limits(applied.qp_min, applied.qp_max);
#endif
        stream->start_looping();
        started = true;
      } else {
#ifdef OPENHD_GSTREAMER_PRESENT
        if (!stream) create_stream();
        gst->queue_worker_settings(snapshot, header.sequence,
                                   header.flags != 0);
        if (!started) {
          stream->start_looping();
          started = true;
        }
#endif
      }
    }
    if (received > 0 && header.type == Type::Control &&
        payload.size() == sizeof(Control)) {
      Control control;
      std::memcpy(&control, payload.data(), sizeof(control));
      if (use_mpp || use_libcamera) {
        if (stream) {
          if (control.bitrate >= 0 &&
              (control.bitrate != applied.bitrate ||
               control.capacity_limit != applied.capacity_limit))
            stream->handle_change_bitrate_request({control.bitrate, false});
          if (control.armed != applied.armed)
            stream->handle_update_arming_state(control.armed);
#ifdef OPENHD_ROCKCHIP_MPP_PRESENT
          if (use_mpp && control.rotation_degree >= 0 && control.flip >= 0 &&
              (control.rotation_degree != applied.rotation_degree ||
               control.flip != applied.flip))
            static_cast<RockchipMppStream*>(stream.get())
                ->set_orientation(control.rotation_degree, control.flip);
          if (use_mpp && control.qp_min >= 0 && control.qp_max >= 0 &&
              (control.qp_min != applied.qp_min ||
               control.qp_max != applied.qp_max))
            static_cast<RockchipMppStream*>(stream.get())
                ->set_qp_limits(control.qp_min, control.qp_max);
          if (use_mpp && control.keyframe_revision != applied.keyframe_revision)
            static_cast<RockchipMppStream*>(stream.get())->request_keyframe();
#endif
        }
        applied = control;
      } else {
#ifdef OPENHD_GSTREAMER_PRESENT
        if (gst) gst->queue_worker_control(control);
#endif
      }
    }
    if (openhd::TerminateHelper::instance().should_terminate())
      std::_Exit(EXIT_FAILURE);
    if (Clock::now() - last_status >= std::chrono::milliseconds(250)) {
      int64_t progress = last_frame_ms.load();
      int recording = AIR_RECORDING_OFF;
      if (stream && use_libcamera)
        recording = static_cast<LibcameraAppStream*>(stream.get())->recording_setting();
#ifdef OPENHD_ROCKCHIP_MPP_PRESENT
      if (stream && use_mpp)
        recording = static_cast<RockchipMppStream*>(stream.get())->recording_setting();
#endif
      unsigned revision = native_revision;
      if ((use_mpp || use_libcamera) && !native_enabled)
        progress = std::chrono::duration_cast<std::chrono::milliseconds>(
                       Clock::now().time_since_epoch())
                       .count();
#ifdef OPENHD_GSTREAMER_PRESENT
      if (gst) {
        progress = gst->last_progress_ms();
        recording = gst->worker_recording_setting();
        revision = gst->worker_settings_revision();
      }
#endif
      Status status{
          openhd::LinkActionHandler::instance().get_cam_info(camera.index),
          progress, recording};
      send_packet(3, {Type::Status, revision}, &status, sizeof(status));
      for (const auto& message :
           openhd::log::MavlinkLogMessageBuffer::instance()
               .dequeue_log_messages())
        send_packet(3, {Type::Log}, &message, sizeof(message));
      last_status = Clock::now();
    }
  }
}
