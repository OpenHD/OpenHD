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

// 0,1 or 2 -> h264,h265 or mjpeg
static bool validate_video_codec(int codec){
  return codec==0 || codec==1 || codec==2;
}

static bool validate_bitrate_mbits(int bitrate_mbits){
  return bitrate_mbits>=1 && bitrate_mbits <=50;
}

static bool validate_camera_rotation(int value){
  return value==0 || value==90 || value==180 || value==270;
}

static bool validate_rpi_awb_mode(int value){
  return value >=0 && value<=9;
}
static bool validate_rpi_exp_mode(int value){
  return value >=0 && value<=12;
}

static bool needs_horizontal_flip(int rotation_value){
  if(rotation_value==180)return true;
  return false;
}
static bool needs_vertical_flip(int rotation_value){
  if(rotation_value==180)return true;
  return false;
}

}
#endif //OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
