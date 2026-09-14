#include "libcamera_app_stream.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/select.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "openhd_spdlog.h"
#include "libcamera_iq_helper.h"
#include "openhd_util.h"

LibcameraAppStream::LibcameraAppStream(
    std::shared_ptr<CameraHolder> camera_holder,
    openhd::ON_ENCODE_FRAME_CB out_cb)
    : CameraStream(std::move(camera_holder), std::move(out_cb)) {
  const bool is_h265 =
      m_camera_holder->get_settings().streamed_video_format.videoCodec ==
      VideoCodec::H265;
  m_rtp = std::make_shared<openhd::RTPHelper>(is_h265);
  m_rtp->set_out_cb([this](auto fragments) {
    if (!this->m_output_cb || fragments.empty()) return;
    const auto& s = this->m_camera_holder->get_settings();
    openhd::FragmentedVideoFrame frame{
        std::move(fragments), std::chrono::steady_clock::now(),
        s.enable_ultra_secure_encryption, nullptr,
        false, false};
    this->m_output_cb(this->m_camera_holder->get_camera().index, frame);
  });
  m_requested_bitrate_kbits = m_camera_holder->get_settings().h26x_bitrate_kbits;
  m_camera_holder->register_listener([this]() { m_restart_requested = true; });
  m_camera_holder->register_video_bitrate_listener([this](int bitrate_kbits) {
    handle_change_bitrate_request({bitrate_kbits});
  });
  m_camera_holder->register_video_qp_listener([this](int qp_min, int qp_max) {
    openhd::log::get_default()->info(
        "Camera{} libcamera QP limits changed to {}-{}; restarting encoder",
        m_camera_holder->get_camera().index, qp_min, qp_max);
    m_restart_requested = true;
  });
}

LibcameraAppStream::~LibcameraAppStream() {
  m_camera_holder->register_video_bitrate_listener(nullptr);
  m_camera_holder->register_video_qp_listener(nullptr);
  terminate_looping();
}

void LibcameraAppStream::start_looping() {
  if (m_run.exchange(true)) return;
  m_thread = std::thread(&LibcameraAppStream::run, this);
}

void LibcameraAppStream::terminate_looping() {
  if (!m_run.exchange(false)) return;
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void LibcameraAppStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  int requested_kbits = lb.recommended_encoder_bitrate_kbits;
  if (lb.is_link_capacity_limit) {
    requested_kbits = std::min(
        requested_kbits,
        m_camera_holder->get_settings().h26x_bitrate_kbits);
  }
  requested_kbits = std::max(
      2000, m_camera_holder->clamp_video_bitrate_kbits(requested_kbits));
  const int previous_kbits = m_requested_bitrate_kbits.exchange(requested_kbits);
  openhd::LinkActionHandler::instance().set_cam_info_bitrate(
      m_camera_holder->get_camera().index,
      static_cast<uint16_t>(requested_kbits));
  if (!lb.is_link_capacity_limit &&
      m_camera_holder->get_settings().h26x_bitrate_kbits != requested_kbits) {
    m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits = requested_kbits;
    m_camera_holder->persist(false);
  }
  if (previous_kbits != requested_kbits) {
    openhd::log::get_default()->info(
        "Camera{} libcamera bitrate request: {} -> {} kbit/s",
        m_camera_holder->get_camera().index, previous_kbits, requested_kbits);
    m_bitrate_update_requested = true;
  }
}

void LibcameraAppStream::handle_update_arming_state(bool armed) {
}

void LibcameraAppStream::run() {
  m_thread_running = true;
  auto log = openhd::log::get_default();
  const char* camera_app = nullptr;
  if (access("/usr/bin/rpicam-vid", X_OK) == 0) {
    camera_app = "/usr/bin/rpicam-vid";
  } else if (access("/usr/bin/libcamera-vid", X_OK) == 0) {
    camera_app = "/usr/bin/libcamera-vid";
  }
  if (!camera_app) {
    log->error("LibcameraAppStream requires rpicam-vid or libcamera-vid");
    m_thread_running = false;
    return;
  }

  while (m_run) {
    m_restart_requested = false;
    const auto settings = m_camera_holder->get_settings();
    const int width = settings.streamed_video_format.width;
    const int height = settings.streamed_video_format.height;
    const int fps = settings.streamed_video_format.framerate;
    const int bitrate_kbits = std::max(2000, m_requested_bitrate_kbits.load());
    const auto& sensor_mode = settings.rpi_libcamera_sensor_mode;
    m_bitrate_update_requested = false;
    const int intra_period = settings.h26x_keyframe_interval > 0
                                 ? settings.h26x_keyframe_interval
                                 : fps;
    const bool is_h265 =
        settings.streamed_video_format.videoCodec == VideoCodec::H265;

    std::stringstream ss;
    ss << camera_app << " -t 0 --inline --nopreview --keypress --codec "
       << (is_h265 ? "h265" : "h264") << " --width " << width
       << " --height " << height << " --framerate " << fps << " ";
    ss << "--mode " << sensor_mode.width << ":" << sensor_mode.height << " ";
    if (!is_h265) ss << "--profile high --level 4.2 ";
    ss << "--bitrate " << bitrate_kbits * 1000 << " --intra " << intra_period
       << " --flush -o -";
    if (!is_h265)
      ss << " --qp-min " << settings.qp_min << " --qp-max "
         << settings.qp_max;
    if (requires_hflip(settings)) ss << " --hflip";
    if (requires_vflip(settings)) ss << " --vflip";
    if (auto value = openhd::libcamera::get_rotation_degree(settings))
      ss << " --rotation " << *value;
    if (auto value = openhd::libcamera::get_brightness(settings))
      ss << " --brightness " << *value;
    if (auto value = openhd::libcamera::get_contrast(settings))
      ss << " --contrast " << *value;
    if (auto value = openhd::libcamera::get_saturation(settings))
      ss << " --saturation " << *value;
    if (auto value = openhd::libcamera::get_sharpness(settings))
      ss << " --sharpness " << *value;
    if (settings.rpi_libcamera_ev_value != RPI_LIBCAMERA_DEFAULT_EV)
      ss << " --ev " << settings.rpi_libcamera_ev_value;
    if (settings.rpi_libcamera_shutter_microseconds != 0)
      ss << " --shutter " << settings.rpi_libcamera_shutter_microseconds;
    const char* awb_modes[] = {"auto", "incandescent", "tungsten",
                               "fluorescent", "indoor", "daylight", "cloudy",
                               "custom"};
    if (settings.rpi_libcamera_awb_index >= 0 &&
        settings.rpi_libcamera_awb_index <= 7)
      ss << " --awb " << awb_modes[settings.rpi_libcamera_awb_index];
    const char* denoise_modes[] = {"auto", "off", "cdn_off", "cdn_fast",
                                   "cdn_hq"};
    if (settings.rpi_libcamera_denoise_index >= 0 &&
        settings.rpi_libcamera_denoise_index <= 4)
      ss << " --denoise " << denoise_modes[settings.rpi_libcamera_denoise_index];
    const char* metering_modes[] = {"centre", "spot", "average", "custom"};
    if (settings.rpi_libcamera_metering_index >= 0 &&
        settings.rpi_libcamera_metering_index <= 3)
      ss << " --metering "
         << metering_modes[settings.rpi_libcamera_metering_index];
    const char* exposure_modes[] = {"normal", "sport"};
    if (settings.rpi_libcamera_exposure_index >= 0 &&
        settings.rpi_libcamera_exposure_index <= 1)
      ss << " --exposure "
         << exposure_modes[settings.rpi_libcamera_exposure_index];

    const auto cam_index = m_camera_holder->get_camera().index;
    openhd::LinkActionHandler::CamInfo cam_info{
        true,
        static_cast<uint8_t>(cam_index),
        static_cast<uint8_t>(m_camera_holder->get_camera().camera_type),
        CAM_STATUS_RESTARTING,
        0,
        static_cast<uint8_t>(
            video_codec_to_int(settings.streamed_video_format.videoCodec)),
        static_cast<uint16_t>(bitrate_kbits),
        static_cast<uint16_t>(bitrate_kbits),
        static_cast<uint8_t>(intra_period),
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        static_cast<uint16_t>(fps),
        0,
        0,
        1,
        static_cast<uint8_t>(settings.qp_max),
        static_cast<uint8_t>(settings.qp_min)};
    openhd::LinkActionHandler::instance().set_cam_info(cam_index, cam_info);
    openhd::LinkActionHandler::instance().set_cam_info_supports_variable_bitrate(
        cam_index, true);

    const std::string cmd = ss.str();
    log->info("LibcameraAppStream starting: {}", cmd);
    int pipe_fds[2] = {-1, -1};
    int control_fds[2] = {-1, -1};
    if (pipe(pipe_fds) != 0 || pipe(control_fds) != 0) {
      log->error("LibcameraAppStream pipe failed: {}", strerror(errno));
      if (pipe_fds[0] >= 0) {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
      }
      break;
    }
    const pid_t child = fork();
    if (child == 0) {
      setpgid(0, 0);
      close(pipe_fds[0]);
      close(control_fds[1]);
      if (dup2(pipe_fds[1], STDOUT_FILENO) < 0) _exit(126);
      if (dup2(control_fds[0], STDIN_FILENO) < 0) _exit(126);
      close(pipe_fds[1]);
      close(control_fds[0]);
      execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char*>(nullptr));
      _exit(127);
    }
    close(pipe_fds[1]);
    if (child < 0) {
      close(pipe_fds[0]);
      close(control_fds[0]);
      close(control_fds[1]);
      log->error("LibcameraAppStream fork failed: {}", strerror(errno));
      break;
    }
    setpgid(child, child);

    const int fd = pipe_fds[0];
    uint8_t buffer[65536];
    std::vector<uint8_t> nalu_buffer;
    uint64_t perf_bytes = 0;
    uint64_t perf_frames = 0;
    auto perf_started = std::chrono::steady_clock::now();
    bool received_video = false;
    bool child_ended = false;

    while (m_run && !m_restart_requested) {
    if (m_bitrate_update_requested.exchange(false)) {
      const int updated_bitrate_kbits = m_requested_bitrate_kbits.load();
      const std::string control_message =
          "b" + std::to_string(updated_bitrate_kbits * 1000) + "\n";
      if (write(control_fds[1], control_message.data(),
                control_message.size()) !=
          static_cast<ssize_t>(control_message.size())) {
        log->warn("Cannot send live bitrate update to rpicam-vid: {}",
                  strerror(errno));
      }
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms timeout
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    int ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(fd, &read_fds)) {
      const ssize_t count = read(fd, buffer, sizeof(buffer));
      if (count > 0) {
        if (!received_video) {
          received_video = true;
          openhd::LinkActionHandler::instance().set_cam_info_status(
              cam_index, CAM_STATUS_STREAMING);
        }
        perf_bytes += static_cast<uint64_t>(count);
        nalu_buffer.insert(nalu_buffer.end(), buffer, buffer + count);
        
        // Extract all complete NALUs
        while (nalu_buffer.size() > 4) {
          // find first start code
          size_t start_idx = 0;
          bool found_start = false;
          for (size_t i = 0; i < nalu_buffer.size() - 2; i++) {
            if (nalu_buffer[i] == 0 && nalu_buffer[i+1] == 0) {
              if (nalu_buffer[i+2] == 1) {
                start_idx = i; found_start = true; break;
              } else if (i + 3 < nalu_buffer.size() && nalu_buffer[i+2] == 0 && nalu_buffer[i+3] == 1) {
                start_idx = i; found_start = true; break;
              }
            }
          }
          
          if (!found_start) {
            // no start code at all, discard all but last 3 bytes (might be part of split start code)
            nalu_buffer.erase(nalu_buffer.begin(), nalu_buffer.end() - 3);
            break;
          }
          
          if (start_idx > 0) {
            // discard junk before first start code
            nalu_buffer.erase(nalu_buffer.begin(), nalu_buffer.begin() + start_idx);
            continue;
          }
          
          // now nalu_buffer starts with a start code. Find the NEXT start code.
          size_t next_start = 0;
          bool found_next = false;
          for (size_t i = 3; i < nalu_buffer.size() - 2; i++) {
            if (nalu_buffer[i] == 0 && nalu_buffer[i+1] == 0) {
              if (nalu_buffer[i+2] == 1) {
                next_start = i; found_next = true; break;
              } else if (i + 3 < nalu_buffer.size() && nalu_buffer[i+2] == 0 && nalu_buffer[i+3] == 1) {
                next_start = i; found_next = true; break;
              }
            }
          }
          
          if (found_next) {
            // we have a full NALU from 0 to next_start
            const size_t start_code_size =
                nalu_buffer.size() >= 4 && nalu_buffer[2] == 0 ? 4 : 3;
            if (next_start > start_code_size) {
              const uint8_t header = nalu_buffer[start_code_size];
              const int nal_type = is_h265 ? ((header >> 1) & 0x3f)
                                           : (header & 0x1f);
              if ((!is_h265 && (nal_type == 1 || nal_type == 5)) ||
                  (is_h265 && nal_type <= 31)) {
                ++perf_frames;
              }
            }
            m_rtp->feed_multiple_nalu(nalu_buffer.data(), next_start);
            nalu_buffer.erase(nalu_buffer.begin(), nalu_buffer.begin() + next_start);
          } else {
            // wait for more data
            break;
          }
        }
      } else {
        log->error("LibcameraAppStream pipe ended or error");
        child_ended = true;
        break;
      }
    } else if (ret < 0) {
      if (errno != EINTR) {
        log->error("LibcameraAppStream select error: {}", strerror(errno));
        break;
      }
    }

      const auto now = std::chrono::steady_clock::now();
      const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  now - perf_started)
                                  .count();
      if (elapsed_ms >= 1000) {
        const auto bitrate_bps = static_cast<uint32_t>(
            perf_bytes * 8ULL * 1000ULL /
            static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
        const auto measured_fps = static_cast<uint16_t>(
            perf_frames * 1000ULL /
            static_cast<uint64_t>(std::max<int64_t>(1, elapsed_ms)));
        openhd::LinkActionHandler::instance().set_cam_info_perf(
            cam_index, bitrate_bps, measured_fps);
        perf_bytes = 0;
        perf_frames = 0;
        perf_started = now;
      }
    }

    close(fd);
    close(control_fds[0]);
    close(control_fds[1]);
    kill(-child, SIGTERM);
    bool reaped = false;
    for (int attempt = 0; attempt < 20; ++attempt) {
      if (waitpid(child, nullptr, WNOHANG) == child) {
        reaped = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!reaped) {
      kill(-child, SIGKILL);
      waitpid(child, nullptr, 0);
    }
    if (!m_run) break;
    if (!m_restart_requested && child_ended) {
      log->warn("LibcameraAppStream encoder exited; restarting");
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  openhd::LinkActionHandler::instance().set_cam_info_status(
      m_camera_holder->get_camera().index, CAM_STATUS_RESTARTING);
  log->info("LibcameraAppStream stopped");
  m_thread_running = false;
}
