//
// Created by consti10 on 19.06.23.
//

#ifndef OPENHD_GST_RECORDER_H
#define OPENHD_GST_RECORDER_H

#include <gst/gstelement.h>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "openhd_spdlog.h"

// TODO for some reason, I cannot make fucking appsrc work !
// Dummy until this issue is resolved
class GstVideoRecorder {
 public:
  GstVideoRecorder();
  ~GstVideoRecorder();
  void enqueue_rtp_fragment(std::shared_ptr<std::vector<uint8_t>> fragment);
  void on_video_data(const uint8_t *data, int data_len);
  void start();
  void stop_and_cleanup();
  bool ready_data = false;

 private:
  std::shared_ptr<spdlog::logger> m_console;
  GstElement *m_gst_pipeline = nullptr;
  GstElement *m_app_src_element = nullptr;
};

#endif  // OPENHD_GST_RECORDER_H
