//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_

#include "openhd-video-frame.h"

namespace openhd{

class ITransmitVideo{
 public:
  /**
   * Video, unidirectional
   * only valid on air (transmit)
   * @param stream_index 0 -> primary video stream, 1 -> secondary video stream
   * @param fragmented_video_frame the "frame" to transmit
   */
  virtual void transmit_video_data(int stream_index,const FragmentedVideoFrame& fragmented_video_frame)=0;
};

class DebugITransmitVideo:public ITransmitVideo{
  void transmit_video_data(int stream_index,const FragmentedVideoFrame& fragmented_video_frame) override{
    openhd::log::get_default()->debug("Stream index:{} N Fragments:{}",stream_index,fragmented_video_frame.frame_fragments.size());
  }
};

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_
