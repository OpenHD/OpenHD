//
// Created by consti10 on 20.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_


// For now, only basic sanity checking on video settings
namespace openhd{

// max: 3840×2160 (4K)
// min: 320x240

static bool validate_video_with(int video_w){
  return video_w >= 320 && video_w<=3840;
}
static bool validate_video_height(int video_h){
  return video_h >= 240 && video_h<=2160;
}

static bool validate_video_fps(int fps){
  return fps>=1 && fps <= 200;
}

// 0,1 or 2
static bool validate_video_format(int format){
  return format==0 || format==1 || format==2;
}

static bool validate_bitrate_mbits(int bitrate_mbits){
  return bitrate_mbits>=1 && bitrate_mbits <=50;
}

}
#endif //OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
