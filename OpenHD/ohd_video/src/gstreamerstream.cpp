/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "gstreamerstream.h"

#include <gst/gst.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include "air_recording_helper.hpp"
#include "config_paths.h"
#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"
#include "gst_recording_demuxer.h"
#include "nalu/CodecConfigFinder.hpp"
#include "nalu/fragment_helper.h"
#include "nalu/nalu_helper.h"
#include "openhd_rtp.h"
#include "openhd_sock.h"
#include "openhd_util.h"
#include "rpi_hdmi_to_csi_v4l2_helper.h"
#include "rtp_eof_helper.h"
#include "spdlog/fmt/bundled/format.h"
#include "x20_cam_helper.h"

namespace {
static constexpr auto kSkipDynamicBitrateLogInterval = std::chrono::seconds(5);
static constexpr int64_t kPerfFirstMessageTimeoutMs = 5000;
static constexpr int64_t kPerfMissingWarningIntervalMs = 10000;
static constexpr int64_t kPerfBitrateWarmupMs = 3000;
static constexpr size_t kPipelineDebugMaxChars = 1200;
static constexpr size_t kPipelineDebugChunkChars = 34;

int64_t steady_clock_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

bool parse_gst_perf_metric(const char* text, const char* key, double& out_value) {
  if (text == nullptr || key == nullptr) {
    return false;
  }
  const auto key_len = std::strlen(key);
  const char* search = text;
  while (search != nullptr) {
    const char* match = std::strstr(search, key);
    if (match == nullptr) {
      return false;
    }
    const char* value_start = match + key_len;
    const bool at_start = match == text;
    const unsigned char prev_char =
        at_start ? static_cast<unsigned char>(' ')
                 : static_cast<unsigned char>(*(match - 1));
    // gst-perf INFO strings may contain similarly named keys like "mean_bps".
    // Ensure we match a standalone metric key (e.g. "; bps: ") rather than a
    // suffix inside another token.
    if (at_start || !(std::isalnum(prev_char) != 0 || prev_char == '_')) {
      char* value_end = nullptr;
      const double parsed = std::strtod(value_start, &value_end);
      if (value_end != value_start) {
        out_value = parsed;
        return true;
      }
    }
    search = match + 1;
  }
  return false;
}

bool parse_gst_perf_bitrate_fps(const char* info_text, uint32_t& bitrate_bps,
                                uint16_t& fps) {
  double parsed_bitrate = 0.0;
  double parsed_fps = 0.0;
  bool has_any_bitrate = false;
  const auto use_bitrate_candidate = [&](const char* key, double scale) {
    double candidate = 0.0;
    if (!parse_gst_perf_metric(info_text, key, candidate)) {
      return;
    }
    has_any_bitrate = true;
    parsed_bitrate = std::max(parsed_bitrate, candidate * scale);
  };
  use_bitrate_candidate("bps: ", 1.0);
  use_bitrate_candidate("bps=", 1.0);
  use_bitrate_candidate("mean_bps: ", 1.0);
  use_bitrate_candidate("mean_bps=", 1.0);
  // Some gst-perf variants report in kbps via bitrate fields.
  use_bitrate_candidate("bitrate: ", 1000.0);
  use_bitrate_candidate("bitrate=", 1000.0);
  use_bitrate_candidate("mean_bitrate: ", 1000.0);
  use_bitrate_candidate("mean_bitrate=", 1000.0);
  if (!has_any_bitrate) {
    return false;
  }
  if (!parse_gst_perf_metric(info_text, "fps: ", parsed_fps)) {
    return false;
  }
  parsed_bitrate = std::max(0.0, parsed_bitrate);
  if (parsed_fps < 0.0) {
    parsed_fps = 0.0;
  }
  const double bitrate_max =
      static_cast<double>(std::numeric_limits<uint32_t>::max());
  const double fps_max = static_cast<double>(std::numeric_limits<uint16_t>::max());
  bitrate_bps = static_cast<uint32_t>(std::llround(std::min(parsed_bitrate, bitrate_max)));
  fps = static_cast<uint16_t>(std::llround(std::min(parsed_fps, fps_max)));
  return true;
}

void send_pipeline_debug_over_mavlink(int cam_index,
                                      const std::string& pipeline) {
  std::string payload = pipeline;
  if (payload.size() > kPipelineDebugMaxChars) {
    payload = payload.substr(0, kPipelineDebugMaxChars - 3) + "...";
  }
  const size_t total =
      std::max<size_t>(1, (payload.size() + kPipelineDebugChunkChars - 1) /
                              kPipelineDebugChunkChars);
  for (size_t seq = 0; seq < total; ++seq) {
    const auto chunk =
        payload.substr(seq * kPipelineDebugChunkChars, kPipelineDebugChunkChars);
    openhd::log::log_via_mavlink(
        static_cast<int>(openhd::log::STATUS_LEVEL::DEBUG),
        fmt::format("OHDPIPE{} {}/{} {}", cam_index, seq, total, chunk));
  }
}
}  // namespace

GStreamerStream::GStreamerStream(std::shared_ptr<CameraHolder> camera_holder,
                                 openhd::ON_ENCODE_FRAME_CB out_cb)
    //: CameraStream(platform, camera_holder, video_udp_port) {
    : CameraStream(std::move(camera_holder), std::move(out_cb)) {
  m_console = openhd::log::create_or_get(
      fmt::format("cam{}", m_camera_holder->get_camera().index));
  assert(m_console);
  m_console->debug("GStreamerStream::GStreamerStream for cam {}",
                   m_camera_holder->get_camera().cam_type_as_verbose_string());
  if (OHDFilesystemUtil::exists(
          (std::string(getConfigBasePath()) + "exp_raw.txt").c_str())) {
    dirty_use_raw = true;
  }
  m_camera_holder->register_listener([this]() {
    m_console->debug(
        "Camera settings changed, requesting pipeline restart "
        "(persisted_h26x_bitrate_kbits:{})",
        m_camera_holder->get_settings().h26x_bitrate_kbits);
    // right now, every time the settings for this camera change, we just
    // re-start the whole stream. That is not ideal, since some cameras support
    // changing for example the bitrate or white balance during operation. But
    // wiring that up is not that easy. We call request_restart() to make sure
    // to not perform heavy operation(s) on the mavlink settings callback, since
    // we need to send the acknowledging response in time. Also, gstreamer and
    // camera(s) are sometimes buggy, so in the worst case gstreamer can become
    // unresponsive and block on the restart operation(s) which would be fatal
    // for telemetry.
    this->request_restart();
  });
  m_camera_holder->register_video_bitrate_listener([this](int bitrate_kbits) {
    openhd::LinkActionHandler::LinkBitrateInformation lb{bitrate_kbits};
    this->handle_change_bitrate_request(lb);
  });
  OHDGstHelper::initGstreamerOrThrow();
  if (m_camera_holder->get_camera().is_camera_type_usb_infiray()) {
    openhd::set_infiray_custom_control_zoom_absolute_async(
        m_camera_holder->get_settings()
            .infiray_custom_control_zoom_absolute_colorpalete,
        m_camera_holder->get_camera().usb_v4l2_device_number);
  }
  // m_gst_video_recorder=std::make_unique<GstVideoRecorder>();
  m_console->debug("GStreamerStream::GStreamerStream done");
}

GStreamerStream::~GStreamerStream() {
  m_camera_holder->register_video_bitrate_listener(nullptr);
  GStreamerStream::terminate_looping();
}

void GStreamerStream::start_looping() {
  {
    std::lock_guard<std::mutex> lock(m_loop_mutex);
    m_loop_exited = false;
  }
  m_keep_looping = true;
  m_loop_thread =
      std::make_unique<std::thread>(&GStreamerStream::loop_infinite, this);
}

void GStreamerStream::terminate_looping() {
  static constexpr auto kJoinTimeout = std::chrono::seconds(5);
  m_keep_looping = false;
  if (m_loop_thread) {
    m_console->debug("Wating for loop thread to terminate");
    bool exited = false;
    {
      std::unique_lock<std::mutex> lock(m_loop_mutex);
      exited = m_loop_cv.wait_for(lock, kJoinTimeout,
                                  [this]() { return m_loop_exited.load(); });
    }
    if (!exited) {
      m_console->error(
          "Loop thread did not exit within {}ms, requesting terminate",
          std::chrono::duration_cast<std::chrono::milliseconds>(kJoinTimeout)
              .count());
      openhd::TerminateHelper::instance().terminate_after(
          "gst_thread_hang", std::chrono::milliseconds(100));
      m_loop_thread->detach();
      m_loop_thread = nullptr;
      return;
    }
    m_loop_thread->join();
    m_loop_thread = nullptr;
  }
}

GstBusSyncReply GStreamerStream::on_gst_bus_message(GstBus* /*bus*/,
                                                    GstMessage* message,
                                                    gpointer user_data) {
  auto* self = static_cast<GStreamerStream*>(user_data);
  if (self != nullptr && message != nullptr) {
    self->handle_gst_message(message);
  }
  return GST_BUS_PASS;
}

void GStreamerStream::handle_gst_message(GstMessage* message) {
  if (message == nullptr) {
    return;
  }
  if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_INFO) {
    return;
  }
  if (GST_MESSAGE_SRC(message) == nullptr) {
    return;
  }
  const char* src_name = GST_OBJECT_NAME(GST_MESSAGE_SRC(message));
  if (src_name == nullptr ||
      std::strcmp(src_name, OHDGstHelper::kEncoderPerfElementName) != 0) {
    return;
  }
  GError* info_error = nullptr;
  gchar* info_debug = nullptr;
  gst_message_parse_info(message, &info_error, &info_debug);
  if (info_debug != nullptr) {
    handle_perf_info_message(info_debug);
  } else if (info_error != nullptr && info_error->message != nullptr) {
    handle_perf_info_message(info_error->message);
  }
  if (info_error != nullptr) {
    g_error_free(info_error);
  }
  if (info_debug != nullptr) {
    g_free(info_debug);
  }
}

bool GStreamerStream::setup_perf_element() {
  cleanup_perf_element();
  m_perf_element = gst_bin_get_by_name(GST_BIN(m_gst_pipeline),
                                       OHDGstHelper::kEncoderPerfElementName);
  if (m_perf_element == nullptr) {
    report_required_perf_problem(
        "gst_perf_missing_element",
        fmt::format("gst-perf is required but the perf element is missing in "
                    "the camera {} pipeline",
                    m_camera_holder->get_camera().index));
    return false;
  }
  if (m_gst_bus == nullptr) {
    report_required_perf_problem(
        "gst_perf_no_bus",
        fmt::format("gst-perf is required but the camera {} pipeline bus is "
                    "unavailable",
                    m_camera_holder->get_camera().index));
    return false;
  }
  gst_bus_set_sync_handler(m_gst_bus, &GStreamerStream::on_gst_bus_message, this,
                           nullptr);
  m_console->debug("Attached gst-perf bus handler on {}",
                   OHDGstHelper::kEncoderPerfElementName);
  return true;
}

void GStreamerStream::cleanup_perf_element() {
  if (m_gst_bus != nullptr) {
    gst_bus_set_sync_handler(m_gst_bus, nullptr, nullptr, nullptr);
  }
  if (m_perf_element != nullptr) {
    gst_object_unref(m_perf_element);
    m_perf_element = nullptr;
  }
}

void GStreamerStream::handle_perf_info_message(const char* info_text) {
  m_console->debug("Perf cam{} raw (gst-perf): {}",
                   m_camera_holder->get_camera().index,
                   info_text == nullptr ? "<null>" : info_text);
  uint32_t bitrate_bps = 0;
  uint16_t fps = 0;
  if (!parse_gst_perf_bitrate_fps(info_text, bitrate_bps, fps)) {
    return;
  }
  const int64_t now_ms = steady_clock_ms();
  if (m_perf_first_message_ms <= 0) {
    m_perf_first_message_ms = now_ms;
  }
  if (bitrate_bps > 0) {
    m_perf_seen_nonzero_bitrate = true;
  } else if (fps > 0 && !m_perf_seen_nonzero_bitrate &&
             now_ms - m_perf_first_message_ms < kPerfBitrateWarmupMs) {
    m_console->debug(
        "Perf cam{} (gst-perf): skipping warmup zero bitrate fps={} age_ms={}",
        m_camera_holder->get_camera().index, fps, now_ms - m_perf_first_message_ms);
    return;
  }
  m_last_perf_message_ms.store(steady_clock_ms(), std::memory_order_relaxed);
  m_console->debug("Perf cam{} (gst-perf): bitrate={} bps fps={}",
                   m_camera_holder->get_camera().index, bitrate_bps, fps);
  openhd::LinkActionHandler::instance().set_cam_info_perf(
      m_camera_holder->get_camera().index, bitrate_bps, fps);
}

void GStreamerStream::check_required_perf_telemetry(int64_t now_ms,
                                                    int64_t first_frame_ms) {
  if (first_frame_ms <= 0) {
    return;
  }
  const int64_t last_perf_ms =
      m_last_perf_message_ms.load(std::memory_order_relaxed);
  const bool never_received = last_perf_ms <= 0;
  const int64_t reference_ms = never_received ? first_frame_ms : last_perf_ms;
  const int64_t timeout_ms = never_received ? kPerfFirstMessageTimeoutMs
                                            : kPerfMissingWarningIntervalMs;
  if (now_ms - reference_ms < timeout_ms) {
    return;
  }
  if (now_ms - m_last_perf_warning_ms < kPerfMissingWarningIntervalMs) {
    return;
  }
  m_last_perf_warning_ms = now_ms;
  const auto cam_index = m_camera_holder->get_camera().index;
  openhd::LinkActionHandler::instance().set_cam_info_perf(cam_index, 0, 0);
  if (never_received) {
    report_required_perf_problem(
        "gst_perf_no_data",
        fmt::format("gst-perf is required but has not reported bitrate/fps "
                    "for camera {}; measured bitrate stays unavailable",
                    cam_index));
  } else {
    report_required_perf_problem(
        "gst_perf_stale",
        fmt::format("gst-perf is required but stopped reporting bitrate/fps "
                    "for camera {}; measured bitrate may be stale",
                    cam_index));
  }
}

void GStreamerStream::report_required_perf_problem(
    const std::string& code, const std::string& description) {
  const int64_t now_ms = steady_clock_ms();
  const int64_t last_ms =
      m_last_required_perf_problem_ms.load(std::memory_order_relaxed);
  if (last_ms > 0 && now_ms - last_ms < kPerfMissingWarningIntervalMs) {
    m_console->debug("{}", description);
    return;
  }
  m_last_required_perf_problem_ms.store(now_ms, std::memory_order_relaxed);
  m_console->error("{}", description);
  openhd::Reporter::instance().report_status(code, description, 10000);
  openhd::log::log_via_mavlink(
      static_cast<int>(openhd::log::STATUS_LEVEL::ERROR),
      fmt::format("cam{} {}", m_camera_holder->get_camera().index, code));
}

bool GStreamerStream::should_skip_runtime_bitrate_update() const {
  const auto& camera = m_camera_holder->get_camera();
  if (camera.requires_rockchip3_mpp_pipeline() ||
      camera.requires_rockchip5_mpp_pipeline() ||
      camera.requires_rockchip_rv_pipeline()) {
    return true;
  }
  return false;
}

std::string GStreamerStream::create_source_encode_pipeline(
    const CameraHolder& cam_holder) {
  const auto& camera = cam_holder.get_camera();
  CameraSettings setting = cam_holder.get_settings();

  const bool RPI_HDMI_TO_CSI_USE_V4l2 = OHDFilesystemUtil::exists(
      std::string(getConfigBasePath()) + "hdmi_v4l2.txt");

  openhd::log::get_default()->debug("RPI_HDMI_TO_CSI_USE_V4l2: {}",
                                    RPI_HDMI_TO_CSI_USE_V4l2);

  if (OHDPlatform::instance().is_x20()) {
    openhd::log::get_default()->debug(
        "Platform is x20. Applying x20 RunCam IQ settings.");
    openhd::x20::apply_x20_runcam_iq_settings(setting);
  } else if (camera.requires_rpi_mmal_pipeline() && RPI_HDMI_TO_CSI_USE_V4l2) {
    openhd::log::get_default()->debug(
        "Initializing resolution for RPI MMAL pipeline with V4l2.");
    openhd::rpi::hdmi::initialize_resolution(
        setting.streamed_video_format.width,
        setting.streamed_video_format.height,
        setting.streamed_video_format.framerate);
  }

  std::stringstream pipeline;

  if (camera.requires_rpi_mmal_pipeline()) {
    openhd::log::get_default()->debug("Camera requires RPI MMAL pipeline.");
    if (RPI_HDMI_TO_CSI_USE_V4l2) {
      openhd::log::get_default()->warn("Using RPI HDMI V4l2 stream.");
      pipeline << OHDGstHelper::create_rpi_hdmi_v4l2_stream(setting);
    } else {
      openhd::log::get_default()->warn("Using RPI Camera source stream.");
      pipeline << OHDGstHelper::createRpicamsrcStream(
          -1, setting, cam_holder.requires_half_bitrate_workaround());
    }
  } else if (camera.requires_rpi_libcamera_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires RPI Libcamera pipeline.");
    pipeline << OHDGstHelper::createLibcamerasrcStream(setting);
  } else if (camera.requires_rpi_veye_pipeline()) {
    openhd::log::get_default()->debug("Camera requires RPI Veye pipeline.");
    auto bus = "/dev/video0";
    pipeline << OHDGstHelper::create_veye_vl2_stream(setting, bus);
  } else if (camera.requires_rockchip3_mpp_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires Rockchip3 MPP pipeline.");
    if (camera.camera_type == X_CAM_TYPE_ROCK_3_HDMI_IN) {
      openhd::log::get_default()->warn("Using Rockchip HDMI stream.");
      pipeline << OHDGstHelper::createRockchipHDMIStream(setting);
    } else {
      openhd::log::get_default()->warn(
          "Determining V4l2 file number for Rockchip platform.");
      const int v4l2_filenumber =
          OHDPlatform::instance().platform_type ==
                      X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W ||
                  OHDPlatform::instance().platform_type ==
                      X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3
              ? 0
              : 0;
      pipeline << OHDGstHelper::createRockchipCSIStream(v4l2_filenumber,
                                                        setting);
    }
  } else if (camera.requires_rockchip5_mpp_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires Rockchip5 MPP pipeline.");
    if (camera.camera_type == X_CAM_TYPE_ROCK_5_HDMI_IN) {
      openhd::log::get_default()->warn("Using Rockchip HDMI stream.");
      pipeline << OHDGstHelper::createRockchipHDMIStream(setting);
    } else {
      const int v4l2_filenumber = 11;
      openhd::log::get_default()->warn(
          "Using V4l2 file number: 11 for Rockchip CSI stream.");
      pipeline << OHDGstHelper::createRockchipCSIStream(v4l2_filenumber,
                                                        setting);
    }
  } else if (camera.requires_x20_cedar_pipeline()) {
    openhd::log::get_default()->debug("Camera requires X20 Cedar pipeline.");
    pipeline << OHDGstHelper::createAllwinnerStream(setting);
  } else if (camera.requires_a733_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires Allwinner A733 CSI pipeline.");
    // On the A733 platform the CSI input is exposed as /dev/video1
    const int sensor_id = 1;
    pipeline << OHDGstHelper::createAllwinnerCsiStream(setting, sensor_id);
  } else if (camera.requires_orqa_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires ORQA pipeline. Using ORQA capture pipeline on "
        "/dev/video3 (io-mode=mmap).");
    pipeline << OHDGstHelper::create_orqa_camera1_stream(3, setting);
  } else if (camera.requires_nxp_imx8_v4l2_pipeline()) {
    openhd::log::get_default()->debug(
        "Camera requires NXP i.MX8 V4L2 pipeline.");
    pipeline << OHDGstHelper::create_nxp_imx8_v4l2_stream(setting);
  } else if (is_usb_camera(camera.camera_type)) {
    openhd::log::get_default()->warn("Detected USB camera.");
    const auto v4l2_device_name =
        get_v4l2_device_name_string(camera.usb_v4l2_device_number);
    pipeline << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(
        v4l2_device_name, setting);
  } else if (camera.camera_type == X_CAM_TYPE_DUMMY_SW) {
    openhd::log::get_default()->warn("Using Dummy SW camera type.");
    pipeline << OHDGstHelper::createDummyStreamX(setting);
  } else if (camera.camera_type == X_CAM_TYPE_EXTERNAL ||
             camera.camera_type == X_CAM_TYPE_EXTERNAL_IP) {
    openhd::log::get_default()->warn(
        "Using external camera or external IP camera.");
    pipeline << OHDGstHelper::create_input_custom_udp_rtp_port(setting);
  } else if (camera.camera_type == X_CAM_TYPE_DEVELOPMENT_FILESRC) {
    openhd::log::get_default()->warn(
        "Using development file source camera type.");
    pipeline << OHDGstHelper::create_dummy_filesrc_stream(setting);
  } else if (camera.camera_type == X_CAM_TYPE_NVIDIA_XAVIER_IMX577) {
    openhd::log::get_default()->warn("Using NVIDIA Xavier IMX577 camera type.");
    pipeline << OHDGstHelper::create_nvidia_xavier_stream(setting);
  } else if (camera.camera_type == X_CAM_TYPE_QC_IMX577) {
    openhd::log::get_default()->warn("Using Qualcomm IMX577 camera type.");
    pipeline << OHDGstHelper::create_qualcomm_camera1_stream(0, setting);
  } else if (camera.camera_type == X_CAM_TYPE_ORQA_HORNET) {
    openhd::log::get_default()->warn("Using ORQA HORNET camera type.");
    pipeline << OHDGstHelper::create_orqa_camera1_stream(3, setting);
  } else if (camera.camera_type == X_CAM_TYPE_ROCKCHIP_RV110X) {
    openhd::log::get_default()->warn("Using Rockchip RV camera type.");
    pipeline << OHDGstHelper::createRv1106Stream(setting);
  } else if (camera.camera_type == X_CAM_TYPE_ROCKCHIP_RV1126) {
    openhd::log::get_default()->warn("Using Rockchip RV1126 camera type.");
    pipeline << OHDGstHelper::createRv1126Stream(setting);
  } else {
    openhd::log::get_default()->warn("UNKNOWN CAMERA TYPE");
    pipeline << OHDGstHelper::createDummyStreamX(setting);
  }

  openhd::log::get_default()->debug("Pipeline created: {}", pipeline.str());
  return pipeline.str();
}

bool GStreamerStream::setup() {
  m_console->debug("GStreamerStream::setup() begin");
  const auto& setting = m_camera_holder->get_settings();
  std::stringstream pipeline_content;
  m_bitrate_ctrl_element = std::nullopt;
  m_last_perf_message_ms.store(0, std::memory_order_relaxed);
  m_last_perf_warning_ms = steady_clock_ms() - kPerfMissingWarningIntervalMs;
  m_perf_first_message_ms = 0;
  m_perf_seen_nonzero_bitrate = false;
  auto* perf_factory = gst_element_factory_find("perf");
  if (perf_factory == nullptr) {
    report_required_perf_problem(
        "gst_perf_missing",
        fmt::format("gst-perf is required for camera {}; install "
                    "gstreamer1.0-plugins-bad",
                    m_camera_holder->get_camera().index));
    return false;
  }
  gst_object_unref(perf_factory);
  pipeline_content << create_source_encode_pipeline(*m_camera_holder);
  pipeline_content << OHDGstHelper::createEncoderPerfElement();
  // quick check,here the pipeline should end with a "! ";
  if (!OHDUtil::endsWith(pipeline_content.str(), "! ")) {
    m_console->warn("Probably ill-formatted pipeline: [{}]",
                    pipeline_content.str());
  }
  const bool ADD_RECORDING_TO_PIPELINE =
      setting.air_recording == AIR_RECORDING_ON ||
      (setting.air_recording == AIR_RECORDING_AUTO_ARM_DISARM &&
       m_armed_enable_air_recording);
  // for safety we only add the tee command at the right place if recording is
  // enabled.
  if (ADD_RECORDING_TO_PIPELINE) {
    m_console->info("Air recording active");
    pipeline_content << "tee name=t ! ";
  }
  // After we've written the parts for the different camera implementation(s) we
  // just need to append the rtp part and the udp out add rtp part
  if (dirty_use_raw) {
    /*pipeline_content <<
    OHDGstHelper::create_queue_and_parse(setting.streamed_video_format.videoCodec);
    pipeline_content <<
    OHDGstHelper::create_caps_nal(setting.streamed_video_format.videoCodec);
    pipeline_content << " queue ! ";*/
    pipeline_content << " queue ! ";
    /*pipeline_content << OHDGstHelper::create_parse_for_codec(
        setting.streamed_video_format.videoCodec);
    pipeline_content << OHDGstHelper::create_caps_nal(
        setting.streamed_video_format.videoCodec, true);*/
    pipeline_content << OHDGstHelper::createOutputAppSink();
    /*pipeline_content << "video/x-h264,stream-format=byte-stream ! ";
    pipeline_content << OHDGstHelper::createOutputAppSink();*/
  } else {
    const int rtp_fragment_size = 1440;
    m_console->debug("Using {} for rtp fragmentation", rtp_fragment_size);
    pipeline_content << OHDGstHelper::create_parse_and_rtp_packetize(
        setting.streamed_video_format.videoCodec, rtp_fragment_size);
    pipeline_content << OHDGstHelper::createOutputAppSink();
  }
  if (ADD_RECORDING_TO_PIPELINE) {
    const auto recording_filename =
        openhd::video::create_unused_recording_filename(
            OHDGstHelper::file_suffix_for_video_codec(
                setting.streamed_video_format.videoCodec));
    m_console->debug("Using [{}] for recording", recording_filename);
    pipeline_content << OHDGstHelper::createRecordingForVideoCodec(
        setting.streamed_video_format.videoCodec, recording_filename);
    m_opt_curr_recording_filename = recording_filename;
  } else {
    m_opt_curr_recording_filename = std::nullopt;
  }
  {
    const auto index = m_camera_holder->get_camera().index;
    const uint8_t cam_type = (uint8_t)m_camera_holder->get_camera().camera_type;
    auto cam_info = openhd::LinkActionHandler::CamInfo{
        true,
        (uint8_t)index,
        cam_type,
        CAM_STATUS_RESTARTING,
        ADD_RECORDING_TO_PIPELINE,
        (uint8_t)video_codec_to_int(setting.streamed_video_format.videoCodec),
        0,
        (uint16_t)setting.h26x_bitrate_kbits,
        (uint8_t)setting.h26x_keyframe_interval,
        (uint16_t)setting.streamed_video_format.width,
        (uint16_t)setting.streamed_video_format.height,
        (uint16_t)setting.streamed_video_format.framerate,
        0,
        0,
        0,
        0};
    openhd::LinkActionHandler::instance().set_cam_info(index, cam_info);
  }
  const auto full_pipeline = pipeline_content.str();
  m_console->debug("Starting pipeline:[{}]", full_pipeline);
  send_pipeline_debug_over_mavlink(m_camera_holder->get_camera().index,
                                   full_pipeline);
  // Protect against unwanted use - stop and free the pipeline first
  assert(m_gst_pipeline == nullptr);
  // Now start the (as a string) built pipeline
  GError* error = nullptr;
  m_gst_pipeline = gst_parse_launch(full_pipeline.c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");
  if (error) {
    report_required_perf_problem(
        "gst_pipeline_parse",
        fmt::format("Failed to create camera {} pipeline: {}",
                    m_camera_holder->get_camera().index, error->message));
    g_error_free(error);
    if (m_gst_pipeline != nullptr) {
      cleanup_pipe();
    }
    return false;
  }
  if (m_gst_pipeline == nullptr) {
    report_required_perf_problem(
        "gst_pipeline_null",
        fmt::format("Failed to create camera {} pipeline",
                    m_camera_holder->get_camera().index));
    return false;
  }
  m_gst_bus = gst_pipeline_get_bus(GST_PIPELINE(m_gst_pipeline));
  if (m_gst_bus == nullptr) {
    report_required_perf_problem(
        "gst_pipeline_no_bus",
        fmt::format("Cannot get GST bus for camera {} pipeline",
                    m_camera_holder->get_camera().index));
    cleanup_pipe();
    return false;
  }
  m_bitrate_ctrl_element = get_dynamic_bitrate_control_element_in_pipeline(
      m_gst_pipeline, *m_camera_holder);
  openhd::LinkActionHandler::instance().set_cam_info_supports_variable_bitrate(
      m_camera_holder->get_camera().index, m_bitrate_ctrl_element.has_value());
  if (!setup_perf_element()) {
    cleanup_pipe();
    return false;
  }
  // we pull data out of the gst pipeline as cpu memory buffer(s) using the
  // gstreamer "appsink" element
  m_app_sink_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "out_appsink");
  if (m_app_sink_element == nullptr) {
    report_required_perf_problem(
        "gst_pipeline_no_appsink",
        fmt::format("Cannot find appsink in camera {} pipeline",
                    m_camera_holder->get_camera().index));
    cleanup_pipe();
    return false;
  }
  // m_console->debug("Cam encoding format: {}",(int)cam_info.encoding_format);
  auto lol_cb =
      [this](
          std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
        x_on_new_rtp_fragmented_frame(frame_fragments);
      };
  m_rtp_helper = std::make_shared<openhd::RTPHelper>(
      setting.streamed_video_format.videoCodec == VideoCodec::H265);
  m_rtp_helper->set_out_cb(lol_cb);
  return true;
}

void GStreamerStream::start() {
  m_console->debug("GStreamerStream::start()");
  assert(m_gst_pipeline != nullptr);
  openhd::register_message_cb(m_gst_pipeline);
  auto ret =
      openhd::gst_element_set_state_with_timeout(m_gst_pipeline,
                                                 GST_STATE_PLAYING);
  if (ret.has_value()) {
    m_console->debug("State change ret:{}",
                     openhd::gst_state_change_return_to_string(ret.value()));
  }
}

void GStreamerStream::stop() {
  m_console->debug("GStreamerStream::stop()");
  assert(m_gst_pipeline != nullptr);
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_PAUSED);
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
}

void GStreamerStream::cleanup_pipe() {
  m_console->debug("GStreamerStream::cleanup_pipe() begin");
  assert(m_gst_pipeline != nullptr);
  cleanup_perf_element();
  if (m_gst_bus != nullptr) {
    gst_object_unref(m_gst_bus);
    m_gst_bus = nullptr;
  }
  // Drop the reference to the bitrate control element (if it exists)
  if (m_bitrate_ctrl_element.has_value()) {
    unref_bitrate_element(m_bitrate_ctrl_element.value());
    m_bitrate_ctrl_element = std::nullopt;
  }
  // As well as the appsink (always exists)
  openhd::unref_appsink_element(m_app_sink_element);
  m_app_sink_element = nullptr;
  // Jan 22: Confirmed this hangs quite a lot of pipeline(s) - removed for that
  // reason
  /*m_console->debug("send EOS begin");
  // according to @Alex W we need a EOS signal here to properly shut down the
  pipeline if(!gst_element_send_event (m_gst_pipeline, gst_event_new_eos())){
    m_console->info("error gst_element_send_event eos"); // No idea what that
  means }else{ m_console->info("success gst_element_send_event eos");
  }*/
  // TODO do we need to wait until the pipeline is actually in state NULL ?
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_NULL);
  openhd::gst_object_unref_with_timeout(GST_OBJECT(m_gst_pipeline));
  m_gst_pipeline = nullptr;
  if (m_opt_curr_recording_filename) {
    // make file read / writeable by everybody
    OHDFilesystemUtil::make_file_read_write_everyone(
        m_opt_curr_recording_filename.value());
    // we do not want empty files - this can happen rarely in case the file is
    // created, but no video data is actually written to it actually, looks like
    // it is possible the file might be empty until the gst pipeline is actually
    // terminating - annoying gst crap ! why is there no way to properly
    // terminate ! better leave the file there
    /*if(OHDFilesystemUtil::get_file_size_bytes(m_opt_curr_recording_filename.value())==0){
      m_console->debug("Ground recording {} is
    empty",m_opt_curr_recording_filename.value());
      OHDFilesystemUtil::remove_if_existing(m_opt_curr_recording_filename.value());
    }*/
    m_opt_curr_recording_filename = std::nullopt;
  }
  // start demuxing of (all) .mkv files unless the FC is currently armed ( we
  // are in flight) this will of course also de-mux the new ground recording (if
  // there is any)
  // if(m_opt_action_handler &&
  // !m_opt_action_handler->arm_state.is_currently_armed()){
  // GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
  //}
  m_console->debug("GStreamerStream::cleanup_pipe() end");
}

void GStreamerStream::request_restart() { m_request_restart = true; }

void GStreamerStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  if (should_skip_runtime_bitrate_update()) {
    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_log_skip_dynamic_bitrate >
        kSkipDynamicBitrateLogInterval) {
      m_last_log_skip_dynamic_bitrate = now;
      m_console->warn(
          "Ignoring runtime bitrate update for Rockchip/MPP camera (requested "
          "{} kBit/s)",
          lb.recommended_encoder_bitrate_kbits);
    }
    return;
  }
  // m_console->debug("handle_change_bitrate_request prev: {} new:{}",
  //                  kbits_per_second_to_string(m_curr_dynamic_bitrate_kbits),
  //                  kbits_per_second_to_string(lb.recommended_encoder_bitrate_kbits));
  //  We do some safety checks first - the link might recommend too much / too
  //  little
  auto bitrate_for_encoder_kbits = lb.recommended_encoder_bitrate_kbits;
  m_console->debug(
      "Bitrate request received cam{}: requested_kbits:{} current_target_kbits:{} "
      "persisted_kbits:{}",
      m_camera_holder->get_camera().index, bitrate_for_encoder_kbits,
      m_curr_dynamic_bitrate_kbits.load(),
      m_camera_holder->get_settings().h26x_bitrate_kbits);
  // m_console->debug(
  //     "Received bitrate update request: {} kBit/s (current target: {}
  //     kBit/s)", bitrate_for_encoder_kbits,
  //     m_curr_dynamic_bitrate_kbits.load());
  static auto MIN_BITRATE_KBITS = 1 * 1000;
  // RPi cannot do less than 2MBit/s
  if (OHDPlatform::instance().is_rpi()) {
    MIN_BITRATE_KBITS = 2 * 1000;
  }
  if (bitrate_for_encoder_kbits < MIN_BITRATE_KBITS) {
    // m_console->debug("Cam cannot do <{}",
    // kbits_per_second_to_string(MIN_BITRATE_KBITS));
    m_console->debug(
        "Clamping bitrate request from {} kBit/s to minimum {} kBit/s",
        bitrate_for_encoder_kbits, MIN_BITRATE_KBITS);
    bitrate_for_encoder_kbits = MIN_BITRATE_KBITS;
  }
  // The gst thread is responsible for changing the bitrate - it will be applied
  // (as long as the cam is not bugged or the OS is overloaded) after a max
  // delay of 40ms
  m_curr_dynamic_bitrate_kbits = bitrate_for_encoder_kbits;
  if (m_camera_holder->get_settings().h26x_bitrate_kbits !=
      bitrate_for_encoder_kbits) {
    m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits =
        bitrate_for_encoder_kbits;
    m_camera_holder->persist(false);
  }
}

void GStreamerStream::handle_update_arming_state(bool armed) {
  m_console->debug("handle_update_arming_state: {}", armed);
  const auto settings = m_camera_holder->get_settings();
  if (settings.air_recording == AIR_RECORDING_AUTO_ARM_DISARM) {
    if (armed) {
      m_armed_enable_air_recording = true;
      m_console->debug("Starting air recording");
    } else {
      m_armed_enable_air_recording = false;
    }
    // restart pipeline such that recording is started / stopped
    request_restart();
  }
}

void GStreamerStream::loop_infinite() {
  while (m_keep_looping) {
    try {
      stream_once();
    } catch (std::exception& ex) {
      std::cerr << "GStreamerStream::Error: " << ex.what() << std::endl;
    } catch (...) {
      std::cerr << "GStreamerStream::Unknown exception occurred" << std::endl;
    }
  }
  {
    std::lock_guard<std::mutex> lock(m_loop_mutex);
    m_loop_exited = true;
  }
  m_loop_cv.notify_all();
}

void GStreamerStream::stream_once() {
  // The user can disable streaming for a camera, in which case a restart is
  // requested and after that we land here (and do nothing)
  if (!m_camera_holder->get_settings().enable_streaming) {
    const auto elapsed_log =
        std::chrono::steady_clock::now() - m_last_log_streaming_disabled;
    if (elapsed_log > std::chrono::seconds(5)) {
      m_console->debug("streaming disabled");
      m_last_log_streaming_disabled = std::chrono::steady_clock::now();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }
  // First, we (try) starting the pipeline using the current settings
  openhd::LinkActionHandler::instance().set_cam_info_status(
      m_camera_holder->get_camera().index, CAM_STATUS_RESTARTING);
  if (!setup()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }
  if (OHDPlatform::instance().is_x20()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  start();
  // Check if we were able to successfully start the pipeline. If - for example
  // - the camera doesn't exist or the resolution set is not supported by the
  // camera, we won't get further than this.
  bool succesfully_streaming = false;
  m_console->debug(openhd::gst_element_get_current_state_as_string(
      m_gst_pipeline, &succesfully_streaming));
  /*if(m_camera_holder->get_camera().rpi_csi_mmal_is_csi_to_hdmi ||
  m_camera_holder->get_camera().type==CameraType::ALLWINNER_CSI){
    m_console->debug("Not checking gst state after calling play (bugged)");
    succesfully_streaming= true;
  }*/
  succesfully_streaming = true;
  if (!succesfully_streaming) {
    m_console->warn("Cannot start streaming. Valid resolution ?",
                    m_camera_holder->get_camera().index);
    stop();
    cleanup_pipe();
    // Sleep a bit and hope it works next time
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }
  //
  // Here we begin the loop where the camera only
  // 1) Constantly produces data
  // 2) reacts to bitrate change(s) from wb link
  // 3) breaks out of if a restart is requested (changed settings) or no data is
  // generated
  //    for X seconds.
  //
  // Bitrate is the only value we (NEED) to support changing without a restart
  int currently_applied_bitrate =
      m_camera_holder->get_settings().h26x_bitrate_kbits;
  m_curr_dynamic_bitrate_kbits = currently_applied_bitrate;
  // Now we should have a running pipeline and are able to pull samples from it
  // We use a timeout of 40ms to not unnecessarily wake up the thread on up to
  // 30fps (33ms) but also quickly respond to restart requests or bitrate
  // change(s)
  const uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::nanoseconds(1000000 * 40))
          .count();
  // For 'bugged camera restart' fix
  std::chrono::steady_clock::time_point m_last_camera_frame =
      std::chrono::steady_clock::now();
  m_frame_fragments.resize(0);
  // As soon as we get the first frame, we change the status to streaming
  bool has_first_frame = false;
  int64_t first_frame_ms = 0;
  // Every X seconds, we check if we are about to run out of space
  std::chrono::steady_clock::time_point
      m_last_air_recording_remaining_space_check =
          std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point m_last_bitrate_diag_log =
      std::chrono::steady_clock::now();
  while (true) {
    const auto loop_now = std::chrono::steady_clock::now();
    const int64_t loop_now_ms = steady_clock_ms();
    // Quickly terminate if openhd wants to terminate
    if (!m_keep_looping) break;
    // ANNOYING BUGGED CAMERAS FIX - we restart the pipeline if we don't get a
    // frame from the camera for more than X seconds
    if (loop_now - m_last_camera_frame > std::chrono::seconds(10)) {
      m_console->warn("Restarting camera due to no frame after 10 seconds");
      openhd::Reporter::instance().report_status(
          "camera_no_frames",
          fmt::format("No frames received from camera {}",
                      m_camera_holder->get_camera().index),
          10000);
      m_request_restart = true;
    }
    // Check if we need to set a new bitrate
    if (currently_applied_bitrate != m_curr_dynamic_bitrate_kbits) {
      const int new_bitrate = m_curr_dynamic_bitrate_kbits;
      m_console->debug("Bitrate change, old:{} new:{}",
                       currently_applied_bitrate, new_bitrate);
      if (m_bitrate_ctrl_element != std::nullopt) {
        // apply the new bitrate
        // Don't forget, the rpi csi hdmi needs the 'half bitrate' hack
        auto hacked_bitrate_kbits = new_bitrate;
        if (m_camera_holder->requires_half_bitrate_workaround()) {
          m_console->debug(
              "applying hack - reduce bitrate by 2 to get actual correct "
              "bitrate");
          hacked_bitrate_kbits = hacked_bitrate_kbits / 2;
        }
        auto bitrate_ctrl_element = m_bitrate_ctrl_element.value();
        if (change_bitrate(bitrate_ctrl_element, hacked_bitrate_kbits)) {
          currently_applied_bitrate = new_bitrate;
          openhd::LinkActionHandler::instance().set_cam_info_bitrate(
              m_camera_holder->get_camera().index, currently_applied_bitrate);
          const auto rb_opt = read_bitrate_readback(bitrate_ctrl_element);
          if (rb_opt.has_value()) {
            int effective_kbits = rb_opt->interpreted_kbits;
            if (m_camera_holder->requires_half_bitrate_workaround()) {
              effective_kbits *= 2;
            }
            m_console->debug(
                "Bitrate apply success cam{}: requested_kbits:{} "
                "applied_kbits:{} encoder_readback_raw:{} "
                "encoder_readback_kbits:{} effective_kbits:{}",
                m_camera_holder->get_camera().index, new_bitrate,
                hacked_bitrate_kbits, rb_opt->raw_property_value,
                rb_opt->interpreted_kbits, effective_kbits);
          }
        } else {
          m_console->warn(
              "Cannot apply bitrate cam{} requested_kbits:{} applied_kbits:{}",
              m_camera_holder->get_camera().index, new_bitrate,
              hacked_bitrate_kbits);
        }
      } else {
        // Sad, but if the camera doesn't support changing the bitrate without a
        // restart, we need to restart
        m_console->info("Bitrate change requires restart (Not good)");
        m_request_restart = true;
      }
    }
    // Check if we require a full restart
    bool tmp_true = true;
    if (m_request_restart.compare_exchange_strong(tmp_true, false)) {
      // Something that requires a whole restart of the pipeline happened
      m_console->debug("Restart requested, restarting");
      break;
    }
    const auto elapsed_remaining_space =
        loop_now - m_last_air_recording_remaining_space_check;
    if (elapsed_remaining_space > std::chrono::seconds(1)) {
      m_camera_holder->check_remaining_space_air_recording(true);
      m_last_air_recording_remaining_space_check = loop_now;
    }
    if (m_bitrate_ctrl_element.has_value() &&
        loop_now - m_last_bitrate_diag_log > std::chrono::seconds(3)) {
      m_last_bitrate_diag_log = loop_now;
      const auto bitrate_ctrl_element = m_bitrate_ctrl_element.value();
      const auto rb_opt = read_bitrate_readback(bitrate_ctrl_element);
      if (rb_opt.has_value()) {
        int effective_kbits = rb_opt->interpreted_kbits;
        if (m_camera_holder->requires_half_bitrate_workaround()) {
          effective_kbits *= 2;
        }
        const int target_kbits = m_curr_dynamic_bitrate_kbits.load();
        const int delta = std::abs(effective_kbits - target_kbits);
        if (delta > 500) {
          m_console->warn(
              "Bitrate mismatch cam{}: target_kbits:{} "
              "encoder_readback_raw:{} encoder_readback_kbits:{} "
              "effective_kbits:{} delta_kbits:{} "
              "persisted_kbits:{} currently_applied_kbits:{}",
              m_camera_holder->get_camera().index, target_kbits,
              rb_opt->raw_property_value, rb_opt->interpreted_kbits,
              effective_kbits, delta,
              m_camera_holder->get_settings().h26x_bitrate_kbits,
              currently_applied_bitrate);
        } else {
          m_console->debug(
              "Bitrate state cam{}: target_kbits:{} "
              "encoder_readback_raw:{} encoder_readback_kbits:{} "
              "effective_kbits:{} persisted_kbits:{} "
              "currently_applied_kbits:{}",
              m_camera_holder->get_camera().index, target_kbits,
              rb_opt->raw_property_value, rb_opt->interpreted_kbits,
              effective_kbits,
              m_camera_holder->get_settings().h26x_bitrate_kbits,
              currently_applied_bitrate);
        }
      } else {
        m_console->warn(
            "Bitrate diagnostics cam{}: cannot read encoder property {}",
            m_camera_holder->get_camera().index,
            bitrate_ctrl_element.property_name);
      }
    }
    if (has_first_frame) {
      check_required_perf_telemetry(loop_now_ms, first_frame_ms);
    }
    // try get a new frame fragment from gst
    GstSample* sample = gst_app_sink_try_pull_sample(
        GST_APP_SINK(m_app_sink_element), timeout_ns);
    if (sample) {
      if (!has_first_frame) {
        has_first_frame = true;
        first_frame_ms = loop_now_ms;
        openhd::LinkActionHandler::instance().set_cam_info_status(
            m_camera_holder->get_camera().index, CAM_STATUS_STREAMING);
      }
      GstBuffer* buffer = gst_sample_get_buffer(sample);
      // tmp declaration for give sample back early optimization
      std::shared_ptr<std::vector<uint8_t>> fragment_data = nullptr;
      uint64_t buffer_dts = 0;
      if (buffer && gst_buffer_get_size(buffer) > 0) {
        fragment_data = openhd::gst_copy_buffer(buffer);
        buffer_dts = buffer->dts;
      }
      // Optimization: Give the buffer back to gstreamer as soon as possible.
      // After copying the data from the sample, unref it first, then forward
      // the data via cb
      gst_sample_unref(sample);
      sample = nullptr;
      if (fragment_data && !fragment_data->empty()) {
        // If we got a new sample, aggregate then forward
        if (dirty_use_raw) {
          m_rtp_helper->feed_multiple_nalu(fragment_data->data(),
                                           fragment_data->size());
        } else {
          on_new_rtp_frame_fragment(std::move(fragment_data), buffer_dts);
        }
        m_last_camera_frame = std::chrono::steady_clock::now();
      }
    }
  }
  // If we land here, we need to clean up the pipe and (re) start
  const auto terminate_begin = std::chrono::steady_clock::now();
  stop();
  cleanup_pipe();
  m_frame_fragments.resize(0);
  m_console->debug("Terminating pipeline took {}ms",
                   std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - terminate_begin)
                       .count());
}

void GStreamerStream::on_new_rtp_frame_fragment(
    std::shared_ptr<std::vector<uint8_t>> fragment, uint64_t dts) {
  m_frame_fragments.push_back(fragment);
  const auto curr_video_codec =
      m_camera_holder->get_settings().streamed_video_format.videoCodec;
  openhd::rtp_eof_helper::RTPFragmentInfo info{};
  const bool is_h265 = curr_video_codec == VideoCodec::H265;
  if (is_h265) {
    info = openhd::rtp_eof_helper::h265_more_info(fragment->data(),
                                                  fragment->size());
  } else {
    info = openhd::rtp_eof_helper::h264_more_info(fragment->data(),
                                                  fragment->size());
  }
  if (info.is_fu_start) {
    if (is_idr_frame(info.nal_unit_type, is_h265)) {
      m_last_fu_s_idr = true;
    } else {
      m_last_fu_s_idr = false;
    }
  }
  // m_console->debug("Fragment {} start:{} end:{}
  // type:{}",m_frame_fragments.size(),
  //                  OHDUtil::yes_or_no(info.is_fu_start),
  //                  OHDUtil::yes_or_no(info.is_fu_end),
  //                  x_get_nal_unit_type_as_string(info.nal_unit_type,is_h265));
  bool is_last_fragment_of_frame = info.is_fu_end;
  if (m_frame_fragments.size() > 500) {
    // Most likely something wrong with the "find end of frame" workaround
    m_console->debug("No end of frame found after 1000 fragments");
    is_last_fragment_of_frame = true;
  }
  if (is_last_fragment_of_frame) {
    on_new_rtp_fragmented_frame();
    m_frame_fragments.resize(0);
    m_last_fu_s_idr = false;
  }
}

void GStreamerStream::on_new_rtp_fragmented_frame() {
  // m_console->debug("Got frame with {} fragments",rtp_fragments.size());
  if (m_output_cb) {
    const auto stream_index = m_camera_holder->get_camera().index;
    const bool enable_ultra_secure_encryption =
        m_camera_holder->get_settings().enable_ultra_secure_encryption;
    const bool is_intra_enabled =
        m_camera_holder->get_settings().h26x_intra_refresh_type != -1;
    const bool is_intra_frame = m_last_fu_s_idr;
    auto frame = openhd::FragmentedVideoFrame{m_frame_fragments,
                                              std::chrono::steady_clock::now(),
                                              enable_ultra_secure_encryption,
                                              nullptr,
                                              is_intra_enabled,
                                              is_intra_frame};
    // m_console->debug("{}",frame.to_string());
    m_output_cb(stream_index, frame);
  } else {
    m_console->debug("No output cb");
  }
}

void GStreamerStream::x_on_new_rtp_fragmented_frame(
    std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
  if (m_output_cb) {
    const auto stream_index = m_camera_holder->get_camera().index;
    const bool enable_ultra_secure_encryption =
        m_camera_holder->get_settings().enable_ultra_secure_encryption;
    const bool is_intra_enabled =
        m_camera_holder->get_settings().h26x_intra_refresh_type != -1;
    const bool is_intra_frame = m_last_fu_s_idr;
    auto frame = openhd::FragmentedVideoFrame{frame_fragments,
                                              std::chrono::steady_clock::now(),
                                              enable_ultra_secure_encryption,
                                              nullptr,
                                              is_intra_enabled,
                                              is_intra_frame};
    // m_console->debug("{}",frame.to_string());
    m_output_cb(stream_index, frame);
  } else {
    m_console->debug("No output cb");
  }
}
