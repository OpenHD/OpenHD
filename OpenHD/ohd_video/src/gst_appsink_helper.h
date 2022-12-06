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
  openhd::log::get_default()->debug(
      "Buffer info[offset:{}, offset_end:{} duration:{} pts:{} dts:{}]",
      buffer->offset,buffer->offset_end,buffer->duration,
      buffer->pts,buffer->dts);
}

}
#endif  // OPENHD_OPENHD_OHD_VIDEO_SRC_GST_APPSINK_HELPER_H_
