//
// Created by consti10 on 17.04.24.
//

#ifndef OPENHD_OPENHD_RTP_H
#define OPENHD_OPENHD_RTP_H


#include "openhd_link.hpp"
#include "openhd_spdlog.h"

#include "rtp-payload-internal.h"

namespace openhd{

class RTPHelper{
 public:
  explicit RTPHelper();


  typedef std::function<void(std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments)> OUT_CB;
  void set_out_cb(RTPHelper::OUT_CB cb);

  void feed_nalu(const uint8_t* data,int data_len);
  // public due to c/c++ mixup
 public:
  void on_new_rtp_fragment(const uint8_t* nalu, int bytes,uint32_t timestamp, int last);
 private:
  OUT_CB m_out_cb= nullptr;
  //openhd::ON_ENCODE_FRAME_CB m_out_cb;
  rtp_payload_t m_handler{};
  void* encoder;
  std::shared_ptr<spdlog::logger> m_console;
 private:
  std::vector<std::shared_ptr<std::vector<uint8_t>>> m_frame_fragments;
};


}

#endif  // OPENHD_OPENHD_RTP_H
