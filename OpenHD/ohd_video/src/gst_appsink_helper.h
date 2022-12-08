//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "openhd-spdlog.hpp"

namespace openhd{

static std::shared_ptr<std::vector<uint8_t>> gst_copy_buffer(GstBuffer* buffer){
  assert(buffer);
  const auto buff_size = gst_buffer_get_size(buffer);
  //openhd::log::get_default()->debug("Got buffer size {}", buff_size);
  auto ret = std::make_shared<std::vector<uint8_t>>(buff_size);
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  assert(map.size == buff_size);
  std::memcpy(ret->data(), map.data, buff_size);
  gst_buffer_unmap(buffer, &map);
  return ret;
}

static void gst_debug_buffer(GstBuffer* buffer){
  assert(buffer);
  const auto now=std::chrono::steady_clock::now().time_since_epoch().count();
  openhd::log::get_default()->debug(
      "Buffer info[offset:{}, offset_end:{} duration:{} pts:{} dts:{} now:{}]",
      buffer->offset,buffer->offset_end,buffer->duration,
      buffer->pts,buffer->dts,now);
}

// based on https://github.com/Samsung/kv2streamer/blob/master/kv2streamer-lib/gst-wrapper/GstAppSinkPipeline.cpp
/**
 * Helper to pull data out of a gstreamer pipeline
 * @param keep_looping if set to false, method returns after max timeout_ns
 * @param app_sink_element the Gst App Sink to pull data from
 * @param out_cb fragments are forwarded via this cb
 */
static void loop_pull_appsink_samples(bool& keep_looping,GstElement *app_sink_element,
                                      const std::function<void(std::shared_ptr<std::vector<uint8_t>> fragment,uint64_t dts)>& out_cb){
  assert(app_sink_element);
  assert(out_cb);
  const uint64_t timeout_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(100)).count();
  while (keep_looping){
    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(app_sink_element),timeout_ns);
    if (sample) {
      //openhd::log::get_default()->debug("Got sample");
      //auto buffer_list=gst_sample_get_buffer_list(sample);
      //openhd::log::get_default()->debug("Got sample {}", gst_buffer_list_length(buffer_list));
      GstBuffer* buffer = gst_sample_get_buffer(sample);
      if (buffer) {
        //openhd::gst_debug_buffer(buffer);
        auto buff_copy=openhd::gst_copy_buffer(buffer);
        //openhd::log::get_default()->debug("Got buffer size {}", buff_copy->size());
        out_cb(buff_copy,buffer->dts);
      }
      gst_sample_unref(sample);
    }
  }
}

}
#endif  // OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_
