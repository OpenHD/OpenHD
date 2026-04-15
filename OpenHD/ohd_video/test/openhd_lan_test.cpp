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

#include <gst/gst.h>

#include <atomic>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "camera_settings.hpp"
#include "gst_helper.hpp"
#include "openhd_util.h"

namespace {

constexpr int kVideoPort = 5600;
constexpr int kRtpMtu = 1440;

std::atomic<bool> g_keep_running{true};

struct Options {
  bool transmit = false;
  bool receive = false;
  bool show_help = false;
  bool use_hello_video = true;
  std::string hello_video_cmd = "fpv_video0.bin /dev/stdin";
  std::optional<std::string> target_ip;
};

void signal_handler(int /*signum*/) { g_keep_running = false; }

void print_usage(const char* app_name) {
  std::cout << "Usage: " << app_name
            << " (-a|-g) [-i <ip>] [--hello-video] "
               "[--hello-video-cmd <cmd>] [--autovideosink]\n"
               "  -a  Transmit H264 RTP stream over UDP (libcamera 720p60)\n"
               "  -g  Receive H264 RTP stream from UDP and display it\n"
               "  -i  Target IP for -a, or bind IP for -g (default: 0.0.0.0)\n";
  std::cout
      << "  --hello-video      Force receive mode through fpv_video0 stdin\n"
      << "  --hello-video-cmd  Command to run "
         "(default: fpv_video0.bin /dev/stdin)\n"
      << "  --autovideosink    Force receive mode through autovideosink\n";
}

std::optional<Options> parse_options(int argc, char* argv[]) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-a") {
      options.transmit = true;
    } else if (arg == "-g") {
      options.receive = true;
    } else if (arg == "-i") {
      if (i + 1 >= argc) {
        std::cerr << "Missing argument for -i\n";
        return std::nullopt;
      }
      const std::string ip = argv[++i];
      if (!OHDUtil::is_valid_ip(ip)) {
        std::cerr << "Invalid IP address: " << ip << "\n";
        return std::nullopt;
      }
      options.target_ip = ip;
    } else if (arg == "--hello-video") {
      options.use_hello_video = true;
    } else if (arg == "--autovideosink") {
      options.use_hello_video = false;
    } else if (arg == "--hello-video-cmd") {
      if (i + 1 >= argc) {
        std::cerr << "Missing argument for --hello-video-cmd\n";
        return std::nullopt;
      }
      options.hello_video_cmd = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      options.show_help = true;
      return options;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return std::nullopt;
    }
  }
  if (options.transmit == options.receive) {
    std::cerr << "Select exactly one mode: -a or -g\n";
    return std::nullopt;
  }
  if (options.transmit && !options.target_ip.has_value()) {
    std::cerr << "-a requires -i <target-ip>\n";
    return std::nullopt;
  }
  return options;
}

std::string create_transmit_pipeline(const std::string& target_ip) {
  CameraSettings settings{};
  settings.streamed_video_format = VideoFormat{VideoCodec::H264, 1280, 720, 60};

  std::stringstream ss;
  ss << OHDGstHelper::createLibcamerasrcStream(settings);
  ss << OHDGstHelper::create_parse_and_rtp_packetize(
      settings.streamed_video_format.videoCodec, kRtpMtu);
  ss << "udpsink host=" << target_ip << " port=" << kVideoPort
     << " sync=false async=false";
  return ss.str();
}

std::string create_receive_pipeline(const std::string& bind_ip) {
  std::stringstream ss;
  ss << "udpsrc address=" << bind_ip << " port=" << kVideoPort << " "
     << OHDGstHelper::gst_create_rtp_caps(VideoCodec::H264) << " ! ";
  ss << OHDGstHelper::create_rtp_depacketize_for_codec(VideoCodec::H264);
  ss << "decodebin ! videoconvert ! autovideosink sync=false";
  return ss.str();
}

std::string create_receive_pipeline_hello_video(const std::string& bind_ip,
                                                int hello_video_fd) {
  std::stringstream ss;
  ss << "udpsrc address=" << bind_ip << " port=" << kVideoPort << " "
     << OHDGstHelper::gst_create_rtp_caps(VideoCodec::H264) << " ! ";
  ss << OHDGstHelper::create_rtp_depacketize_for_codec(VideoCodec::H264);
  ss << "video/x-h264,stream-format=byte-stream ! fdsink fd="
     << hello_video_fd << " sync=false async=false";
  return ss.str();
}

bool run_pipeline_until_signal(const std::string& pipeline) {
  GError* error = nullptr;
  auto* gst_pipeline = gst_parse_launch(pipeline.c_str(), &error);
  if (error != nullptr) {
    std::cerr << "Failed to parse pipeline: " << error->message << "\n";
    g_error_free(error);
    return false;
  }
  if (gst_pipeline == nullptr) {
    std::cerr << "Failed to create pipeline\n";
    return false;
  }

  gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
  auto* bus = gst_element_get_bus(gst_pipeline);
  bool success = true;
  while (g_keep_running) {
    auto* message = gst_bus_timed_pop_filtered(
        bus, 200 * GST_MSECOND,
        static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (message == nullptr) {
      continue;
    }
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
      gst_message_unref(message);
      break;
    }
    GError* gst_error = nullptr;
    gchar* debug = nullptr;
    gst_message_parse_error(message, &gst_error, &debug);
    std::cerr << "GStreamer runtime error: "
              << (gst_error ? gst_error->message : "unknown") << "\n";
    if (debug != nullptr) {
      std::cerr << "Details: " << debug << "\n";
    }
    if (gst_error != nullptr) {
      g_error_free(gst_error);
    }
    if (debug != nullptr) {
      g_free(debug);
    }
    gst_message_unref(message);
    success = false;
    break;
  }

  gst_element_set_state(gst_pipeline, GST_STATE_NULL);
  gst_object_unref(bus);
  gst_object_unref(gst_pipeline);
  return success;
}

bool run_receive_with_hello_video(const std::string& bind_ip,
                                  const std::string& hello_video_cmd) {
  FILE* hello_video_stdin = popen(hello_video_cmd.c_str(), "w");
  if (hello_video_stdin == nullptr) {
    std::cerr << "Failed to start hello_video command: " << hello_video_cmd
              << "\n";
    return false;
  }
  const int hello_video_fd = fileno(hello_video_stdin);
  if (hello_video_fd < 0) {
    std::cerr << "Failed to get hello_video stdin fd\n";
    pclose(hello_video_stdin);
    return false;
  }
  const auto pipeline =
      create_receive_pipeline_hello_video(bind_ip, hello_video_fd);
  const bool success = run_pipeline_until_signal(pipeline);
  const int hello_video_ret = pclose(hello_video_stdin);
  if (hello_video_ret != 0) {
    std::cerr << "hello_video exited with code " << hello_video_ret << "\n";
  }
  return success;
}

}  // namespace

int main(int argc, char* argv[]) {
  const auto options_opt = parse_options(argc, argv);
  if (!options_opt.has_value()) {
    print_usage(argv[0]);
    return 1;
  }
  const auto options = options_opt.value();
  if (options.show_help) {
    print_usage(argv[0]);
    return 0;
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  try {
    OHDGstHelper::initGstreamerOrThrow();
  } catch (const std::exception& ex) {
    std::cerr << "Failed to initialize gstreamer: " << ex.what() << "\n";
    return 1;
  }

  std::string pipeline;
  if (options.transmit) {
    pipeline = create_transmit_pipeline(options.target_ip.value());
    std::cout << "Transmit mode: sending RTP/UDP to "
              << options.target_ip.value() << ":" << kVideoPort << "\n";
    std::cout << "Pipeline: " << pipeline << "\n";
    std::cout << "Press Ctrl+C to stop.\n";
    return run_pipeline_until_signal(pipeline) ? 0 : 1;
  } else {
    const auto bind_ip = options.target_ip.value_or("0.0.0.0");
    if (options.use_hello_video) {
      std::cout << "Receive mode (hello_video): listening on " << bind_ip
                << ":" << kVideoPort << "\n";
      std::cout << "Running command: " << options.hello_video_cmd << "\n";
      std::cout << "Press Ctrl+C to stop.\n";
      return run_receive_with_hello_video(bind_ip, options.hello_video_cmd)
                 ? 0
                 : 1;
    }
    pipeline = create_receive_pipeline(bind_ip);
    std::cout << "Receive mode: listening on " << bind_ip << ":" << kVideoPort
              << "\n";
    std::cout << "Pipeline: " << pipeline << "\n";
    std::cout << "Press Ctrl+C to stop.\n";
    return run_pipeline_until_signal(pipeline) ? 0 : 1;
  }
}
