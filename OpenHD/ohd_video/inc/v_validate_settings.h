//
// Created by consti10 on 20.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_

#include <optional>
#include "openhd-spdlog.hpp"

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
// min: 1 fps max: 240 fps
static bool validate_video_fps(int fps){
  return fps>=1 && fps <= 240;
}

static bool validate_video_width_height_fps(int video_w,int video_h,int fps){
  return validate_video_with(video_w) && validate_video_height(video_h) && validate_video_fps(fps);
}

// 0,1 or 2 -> h264,h265 or mjpeg
static bool validate_video_codec(int codec){
  return codec==0 || codec==1 || codec==2;
}

static bool validate_bitrate_mbits(int bitrate_mbits){
  const bool ret=bitrate_mbits>=1 && bitrate_mbits <=50;
  if(!ret){
    openhd::log::get_default()->warn("Invalid bitrate_mbits: {}",bitrate_mbits);
  }
  return ret;
}

static bool validate_camera_rotation(int value){
  const bool ret= value==0 || value==90 || value==180 || value==270;
  if(!ret){
    openhd::log::get_default()->warn("Invalid camera_rotation: {}",value);
  }
  return ret;
}

static bool validate_rpi_awb_mode(int value){
  return value >=0 && value<=9;
}
static bool validate_rpi_exp_mode(int value){
  return value >=0 && value<=12;
}

// from gst-rpicamsrc: keyframe-interval   : Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe
static bool validate_rpi_keyframe_interval(int value){
  const bool ret=value>=-1 && value < 2147483647;
  if(!ret){
    openhd::log::get_default()->warn("Invalid rpi_keyframe_interval: {}",value);
  }
  return ret;
}

// see gst-rpicamsrc documentation
static bool validate_rpi_intra_refresh_type(int value){
  const bool ret=(value>-1 && value<=2) || value==2130706433;
  if(!ret){
    openhd::log::get_default()->warn("Invalid intra_refresh_type: {}",value);
  }
  return ret;
}


static bool validate_mjpeg_quality_percent(int value){
  const bool ret=value<=100 && value>=1;
  if(!ret){
    openhd::log::get_default()->warn("Invalid mjpeg_quality_percent: {}",value);
  }
  return ret;
}

struct TmpVideoFormat{
  int width_px;
  int height_px;
  int framerate;
};
static std::string video_format_from_int_values(int width,int height,int framerate){
  std::stringstream ss;
  ss<<width<<"x"<<height<<"@"<<framerate;
  return ss.str();
}
// Takes a string in the from {width}x{height}@{framerate}
// e.g. 1280x720@30
static std::optional<TmpVideoFormat> parse_video_format(const std::string& videoFormat){
  if(videoFormat.size()<=5){
	return std::nullopt;
  }
  openhd::log::get_default()->debug("Parsing:["+videoFormat+"]");
  TmpVideoFormat tmp_video_format{0,0,0};
  const std::regex reg{R"((\d*)x(\d*)\@(\d*))"};
  std::smatch result;
  if (std::regex_search(videoFormat, result, reg)) {
	if (result.size() == 4) {
	  openhd::log::get_default()->debug("result[0]=["+result[0].str()+"]");
	  tmp_video_format.width_px=atoi(result[1].str().c_str());
	  tmp_video_format.height_px=atoi(result[2].str().c_str());
	  tmp_video_format.framerate=atoi(result[3].str().c_str());
	  if(validate_video_with(tmp_video_format.width_px) &&
	  	validate_video_height(tmp_video_format.height_px)&& validate_video_fps(tmp_video_format.framerate)){
		std::stringstream log;
		log<<"Final result:{"<<video_format_from_int_values(tmp_video_format.width_px,tmp_video_format.height_px,tmp_video_format.framerate);
		log<<"}";
		openhd::log::get_default()->debug(log.str());
		return tmp_video_format;
	  }
	}
  }
  return std::nullopt;
}


}
#endif //OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
