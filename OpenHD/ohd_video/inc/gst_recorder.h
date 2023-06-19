//
// Created by consti10 on 19.06.23.
//

#ifndef OPENHD_GST_RECORDER_H
#define OPENHD_GST_RECORDER_H

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

// TODO
class GstVideoRecorder {
 public:
  void enqueue_rtp_fragment(std::shared_ptr<std::vector<uint8_t>> fragment);
  void on_video_data(int codec,const uint8_t *data,int data_len);
 private:
  int m_curr_codec=-1;
};

#endif  // OPENHD_GST_RECORDER_H
