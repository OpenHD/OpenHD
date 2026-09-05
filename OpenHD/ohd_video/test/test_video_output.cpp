// Standalone integration test; no camera or OpenHD service restart required.
// g++ -std=c++17 -O2 -pthread -I../inc ../src/gst_video_output.cpp test_video_output.cpp $(pkg-config --cflags --libs gstreamer-app-1.0) -latomic -o test_video_output
#include "gst_video_output.h"
#include <gst/app/gstappsink.h>
#include <chrono>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
  gst_init(&argc, &argv);
  const bool direct_rtp = argc > 1 && std::string(argv[1]) == "rtp";
  const bool raw = argc < 2 || std::string(argv[1]) == "raw";
  GError* error = nullptr;
  auto* receiver = gst_parse_launch("udpsrc name=rtp port=15600 caps=\"application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96\" ! rtpjitterbuffer latency=40 ! rtph264depay ! h264parse ! avdec_h264 ! appsink name=decoded sync=false max-buffers=2 drop=true", &error);
  if (error || !receiver) { std::cerr << (error ? error->message : "No receiver") << '\n'; return 1; }
  const std::string encoder = "x264enc tune=zerolatency speed-preset=ultrafast bitrate=4000 threads=2 key-int-max=30";
  const std::string source_text = "videotestsrc is-live=true pattern=ball ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! " + encoder + " ! h264parse ! rtph264pay pt=96 config-interval=-1 ! appsink name=out_appsink sync=false max-buffers=1 drop=true";
  auto* source = gst_parse_launch(source_text.c_str(), &error);
  if (error || !source) { std::cerr << (error ? error->message : "No source") << '\n'; return 1; }
  std::atomic<uint64_t> bytes{0};
  auto* udp = gst_bin_get_by_name(GST_BIN(receiver), "rtp");
  auto* pad = gst_element_get_static_pad(udp, "src");
  gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, [](GstPad*, GstPadProbeInfo* info, gpointer user) {
    static_cast<std::atomic<uint64_t>*>(user)->fetch_add(gst_buffer_get_size(GST_PAD_PROBE_INFO_BUFFER(info)));
    return GST_PAD_PROBE_OK;
  }, &bytes, nullptr);
  gst_object_unref(pad); gst_object_unref(udp);
  auto output = std::make_unique<openhd::GstVideoOutput>(openhd::VideoOutputProfile{"test", "127.0.0.1", 15600});
  if (direct_rtp) {
    if (!output->start_rtp_input()) return 2;
    auto* encoded_sink = gst_bin_get_by_name(GST_BIN(source), "out_appsink");
    auto* encoded_pad = gst_element_get_static_pad(encoded_sink, "sink");
    gst_pad_add_probe(encoded_pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), [](GstPad*, GstPadProbeInfo* info, gpointer user) {
      auto push = [&](GstBuffer* buffer) {
        GstMapInfo map{};
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
          static_cast<openhd::GstVideoOutput*>(user)->push_rtp(map.data, map.size);
          gst_buffer_unmap(buffer, &map);
        }
      };
      if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) push(GST_PAD_PROBE_INFO_BUFFER(info));
      if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER_LIST) {
        auto* list = GST_PAD_PROBE_INFO_BUFFER_LIST(info);
        for (guint i = 0; i < gst_buffer_list_length(list); ++i) push(gst_buffer_list_get(list, i));
      }
      return GST_PAD_PROBE_OK;
    }, output.get(), nullptr);
    gst_object_unref(encoded_pad); gst_object_unref(encoded_sink);
  } else if (!output->attach(source, false, true, raw) || output->uses_raw_input() != raw) return 2;
  auto* sink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(receiver), "decoded"));
  gst_element_set_state(receiver, GST_STATE_PLAYING);
  gst_element_set_state(source, GST_STATE_PLAYING);
  auto start = std::chrono::steady_clock::now();
  auto first = start;
  int frames = 0, width = 0, height = 0;
  uint64_t initial_bytes = 0;
  while (std::chrono::steady_clock::now() - start < std::chrono::seconds(12)) {
    auto* sample = gst_app_sink_try_pull_sample(sink, GST_SECOND);
    if (!sample) {
      auto* source_bus = gst_element_get_bus(source);
      if (auto* message = gst_bus_pop_filtered(source_bus, GST_MESSAGE_ERROR)) {
        GError* issue = nullptr; gchar* debug = nullptr; gst_message_parse_error(message, &issue, &debug);
        std::cerr << "Primary camera error: " << issue->message << " " << (debug ? debug : "") << '\n';
        g_error_free(issue); g_free(debug); gst_message_unref(message);
      }
      gst_object_unref(source_bus);
      continue;
    }
    auto* caps = gst_sample_get_caps(sample);
    auto* structure = gst_caps_get_structure(caps, 0);
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    if (!frames) { first = std::chrono::steady_clock::now(); initial_bytes = bytes.load(); }
    frames++;
    gst_sample_unref(sample);
  }
  const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - first).count();
  const double fps = (frames - 1) / elapsed;
  const double bitrate = (bytes.load() - initial_bytes) * 8 / elapsed / 1000;
  std::cout << (raw ? "raw" : "encoded") << ": " << width << 'x' << height << ", " << fps << " fps, " << bitrate << " kbit/s RTP, " << frames << " decoded frames\n";
  gst_element_set_state(source, GST_STATE_NULL);
  output.reset();
  gst_element_set_state(receiver, GST_STATE_NULL);
  gst_object_unref(sink); gst_object_unref(source); gst_object_unref(receiver);
  return width == 854 && height == 480 && fps >= 13.5 && fps <= 15.5 && bitrate >= 850 && bitrate <= 1150 ? 0 : 3;
}
