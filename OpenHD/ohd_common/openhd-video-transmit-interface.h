//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_

#include "openhd-video-frame.h"

namespace openhd{

class ITransmitVideo{
 public:
  // Video, unidirectional
  // only valid on air (transmit)
  virtual void transmit_video_data(int stream_index,const FragmentedVideoFrame& fragmented_video_frame)=0;
};

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_VIDEO_TRANSMIT_INTERFACE_H_
