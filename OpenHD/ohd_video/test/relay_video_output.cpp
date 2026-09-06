// Bench receiver for real OpenHD RTP; this tool never generates camera images.
#include "gst_video_output.h"
#include <csignal>
#include <iostream>

namespace { volatile std::sig_atomic_t running = 1; }
int main(int argc, char** argv) {
  gst_init(&argc, &argv);
  if (argc != 5 || (std::string(argv[4]) != "air" && std::string(argv[4]) != "ground")) {
    std::cerr << "Usage: relay_video_output HOST PORT INPUT_PORT air|ground\n";
    return 1;
  }
  int port, input_port;
  try { port = std::stoi(argv[2]); input_port = std::stoi(argv[3]); }
  catch (const std::exception&) { return 1; }
  if (port < 1024 || port > 65535 || input_port < 1024 || input_port > 65535) return 1;
  std::signal(SIGTERM, [](int) { running = 0; });
  std::signal(SIGINT, [](int) { running = 0; });
  const std::string pipeline = "udpsrc address=127.0.0.1 port=" + std::to_string(input_port) +
      " buffer-size=2097152 caps=\"application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96\""
      " ! rtpjitterbuffer latency=100 drop-on-latency=true"
      " ! appsink name=out_appsink sync=false max-buffers=128 drop=true";
  GError* error = nullptr;
  auto* source = gst_parse_launch(pipeline.c_str(), &error);
  if (error || !source) {
    std::cerr << (error ? error->message : "No RTP receiver") << '\n';
    if (error) g_error_free(error);
    if (source) gst_object_unref(source);
    return 2;
  }
  int result = 0;
  {
    openhd::VideoOutputProfile profile{"fleet-relay", argv[1], port};
    profile.fleet_controlled = true;
    openhd::GstVideoOutput output(profile);
    if (!output.attach(source, false, true, false)) {
      gst_object_unref(source);
      return 3;
    }
    gst_element_set_state(source, GST_STATE_PLAYING);
    auto* bus = gst_element_get_bus(source);
    while (running) {
      auto* message = gst_bus_timed_pop_filtered(bus, GST_SECOND, GST_MESSAGE_ERROR);
      if (!message) continue;
      GError* issue = nullptr; gchar* debug = nullptr;
      gst_message_parse_error(message, &issue, &debug);
      std::cerr << issue->message << '\n';
      g_error_free(issue); g_free(debug); gst_message_unref(message);
      result = 4; break;
    }
    gst_object_unref(bus);
    gst_element_set_state(source, GST_STATE_NULL);
  }
  gst_object_unref(source);
  return result;
}
