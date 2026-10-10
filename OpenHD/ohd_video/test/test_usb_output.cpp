#include <gst/app/gstappsink.h>

#include <stdexcept>
#include <cstdio>

#include "gst_helper.hpp"

static void require(bool ok) {
  if (!ok) throw std::runtime_error("USB output regression");
}

// Model a fixed low-resolution capture mode independently of output settings.
// Exercise the real USB builder with common IR rates and a 9 Hz stress case.
static GstElement* make_pipeline(int width, int height, int fps, int capture_fps = 9) {
  CameraSettings settings;
  settings.streamed_video_format = VideoFormat{VideoCodec::H264, width, height, fps};
  settings.h26x_bitrate_kbits = 1000;
  auto pipeline = OHDGstHelper::createV4l2SrcRawAndSwEncodeStream("/dev/video0", settings);
  const auto capture_end = pipeline.find(" ! ");
  require(capture_end != std::string::npos);
  pipeline.replace(0, capture_end,
      "videotestsrc is-live=true ! video/x-raw,width=160,height=128,framerate=" +
      std::to_string(capture_fps) + "/1 ! identity");
  pipeline += "appsink name=test_sink sync=false max-buffers=2 drop=true";
  GError* error = nullptr;
  auto* result = gst_parse_launch(pipeline.c_str(), &error);
  if (error != nullptr) fprintf(stderr, "%s\n", error->message);
  require(result != nullptr && error == nullptr);
  require(gst_element_set_state(result, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE);
  return result;
}

static void check_frame(GstElement* pipeline, int width, int height, int fps) {
  auto* sink = gst_bin_get_by_name(GST_BIN(pipeline), "test_sink");
  auto* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(sink), 3 * GST_SECOND);
  require(sample != nullptr);
  auto* structure = gst_caps_get_structure(gst_sample_get_caps(sample), 0);
  int w = 0, h = 0, numerator = 0, denominator = 0;
  require(gst_structure_get_int(structure, "width", &w));
  require(gst_structure_get_int(structure, "height", &h));
  require(gst_structure_get_fraction(structure, "framerate", &numerator, &denominator));
  require(w == width && h == height && numerator == fps * denominator);
  gst_sample_unref(sample);
  gst_object_unref(sink);
}

int main() {
  gst_init(nullptr, nullptr);
  auto* primary = make_pipeline(160, 128, 9);
  for (const auto capture_fps : {9, 25, 30, 60}) {
    const int output_fps = capture_fps == 9 ? 30 : capture_fps;
    for (const auto width : {320, 640, 160, 320}) {
      const int height = width * 4 / 5;
      auto* secondary = make_pipeline(width, height, output_fps, capture_fps);
      auto* encoder = gst_bin_get_by_name(GST_BIN(secondary), "swencoder");
      for (const guint bitrate : {1000, 1500, 700, 1000}) {
        g_object_set(encoder, "bitrate", bitrate, nullptr);
        guint readback = 0;
        g_object_get(encoder, "bitrate", &readback, nullptr);
        require(readback == bitrate);
        check_frame(secondary, width, height, output_fps);
        check_frame(primary, 160, 128, 9);
      }
      require(gst_element_set_state(secondary, GST_STATE_NULL) != GST_STATE_CHANGE_FAILURE);
      gst_object_unref(encoder);
      gst_object_unref(secondary);
    }
  }
  gst_element_set_state(primary, GST_STATE_NULL);
  gst_object_unref(primary);
}
