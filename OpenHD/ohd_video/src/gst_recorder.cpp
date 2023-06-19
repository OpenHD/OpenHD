//
// Created by consti10 on 19.06.23.
//

#include "gst_recorder.h"

#include <string>

#include "camera_enums.hpp"
#include "gst_helper.hpp"

static std::string create_recording_pipeline(const VideoCodec videoCodec,const std::string& out_filename){
  std::stringstream ss;
  // input - appsrc
  ss<<"appsrc ! ";
  // first de-packetize rtp
  ss<<OHDGstHelper::create_rtp_depacketize_for_codec(videoCodec);
  // parse just to be safe
  ss<<OHDGstHelper::create_parse_for_codec(videoCodec);
  if(videoCodec==VideoCodec::H264 || videoCodec==VideoCodec::H265){
    ss <<"matroskamux ! filesink location="<<out_filename;
  }else{
    ss <<"avimux ! filesink location="<<out_filename;
  }
  return ss.str();
}

void GstVideoRecorder::on_video_data(int codec, const uint8_t *data,
                                     int data_len) {

}
