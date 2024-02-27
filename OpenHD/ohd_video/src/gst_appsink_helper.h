//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include <optional>

#include "openhd_spdlog.h"

namespace openhd {

static std::shared_ptr<std::vector<uint8_t>> gst_copy_buffer(
    GstBuffer* buffer) {
  assert(buffer);
  const auto buff_size = gst_buffer_get_size(buffer);
  // openhd::log::get_default()->debug("Got buffer size {}", buff_size);
  // auto ret = std::make_shared<std::vector<uint8_t>>(buff_size);
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  assert(map.size == buff_size);
  // std::memcpy(ret->data(), map.data, buff_size);
  auto ret =
      std::make_shared<std::vector<uint8_t>>(map.data, map.data + buff_size);
  gst_buffer_unmap(buffer, &map);
  return ret;
}

struct GstBufferX {
  std::shared_ptr<std::vector<uint8_t>> buffer;
  uint64_t buffer_dts = 0;
};
static std::optional<GstBufferX> gst_app_sink_try_pull_sample_and_copy(
    GstElement* app_sink, uint64_t timeout_ns) {
  GstSample* sample =
      gst_app_sink_try_pull_sample(GST_APP_SINK(app_sink), timeout_ns);
  if (!sample) return std::nullopt;
  GstBuffer* buffer = gst_sample_get_buffer(sample);
  // tmp declaration for give sample back early optimization
  std::shared_ptr<std::vector<uint8_t>> fragment_data = nullptr;
  uint64_t buffer_dts = 0;
  if (buffer && gst_buffer_get_size(buffer) > 0) {
    fragment_data = openhd::gst_copy_buffer(buffer);
    buffer_dts = buffer->dts;
  }
  gst_sample_unref(sample);
  if (!fragment_data) return std::nullopt;
  return GstBufferX{fragment_data, buffer_dts};
}

static void gst_debug_buffer(GstBuffer* buffer) {
  assert(buffer);
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  openhd::log::get_default()->debug(
      "Buffer info[offset:{}, offset_end:{} duration:{} pts:{} dts:{} now:{}]",
      buffer->offset, buffer->offset_end, buffer->duration, buffer->pts,
      buffer->dts, now);
}

// From https://github.com/mshabunin/gstreamer-example/blob/master/main.cpp
static void gst_debug_sample(GstSample* sample) {
  assert(sample);
  std::stringstream ss;
  const GstSegment* seg = gst_sample_get_segment(sample);
  ss << "  segment: " << seg << std::endl;
  if (seg) {
    ss << "    Flags: " << seg->flags << std::endl;
    ss << "    Rate: " << seg->rate << std::endl;
    ss << "    Applied rate: " << seg->applied_rate << std::endl;
    ss << "    Format: " << seg->format << std::endl;
    ss << "    Base: " << seg->base << std::endl;
    ss << "    Offset: " << seg->offset << std::endl;
    ss << "    Start: " << seg->start << std::endl;
    ss << "    Stop: " << seg->stop << std::endl;
    ss << "    Time: " << seg->time << std::endl;
    ss << "    Position: " << seg->position << std::endl;
    ss << "    Duration: " << seg->duration << std::endl;
  }
  const GstStructure* info = gst_sample_get_info(sample);
  ss << "  info: " << info << std::endl;
  if (info) {
    gchar* str = gst_structure_to_string(info);
    ss << "    " << str << std::endl;
    g_free(str);
  }
  GstCaps* caps = gst_sample_get_caps(sample);
  ss << "  caps: " << caps << std::endl;
  if (caps) {
    gchar* str = gst_caps_to_string(caps);
    ss << "    " << str << std::endl;
    g_free(str);
  }
  openhd::log::get_default()->debug("{}", ss.str());
}

static void unref_appsink_element(GstElement* appsink) {
  if (appsink) {
    openhd::log::get_default()->debug("Unref appsink begin");
    gst_object_unref(appsink);
    appsink = nullptr;
    openhd::log::get_default()->debug("Unref appsink end");
  }
}

}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_
