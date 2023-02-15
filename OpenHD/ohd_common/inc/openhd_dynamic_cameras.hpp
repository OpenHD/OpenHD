//
// Created by consti10 on 15.02.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_DYNAMIC_CAMERAS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_DYNAMIC_CAMERAS_HPP_

#include <cstdint>

// This class exists to facilitate the need of ohd_interface (link) having to talk to ohd_video
// While there is no code dependencies between them
namespace openhd{

class IDynamicCamera{
 public:
  // Return bitrate manually set by user or -1 if dynamic bitrate control is wanted.
  virtual int get_current_bitrate_kbits()=0;
  //
  struct LinkBitrateInformation{
    uint32_t recommended_encoder_bitrate_kbits;
  };
  virtual void set_dynamic_bitrate(int bitrate)=0;
};

}

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_DYNAMIC_CAMERAS_HPP_
