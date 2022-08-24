//
// Created by consti10 on 20.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_

#include <optional>

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

// from gst-rpicamsrc: keyframe-interval   : Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe
static bool validate_rpi_keyframe_interval(int value){
  return value>=-1 && value < 1000;
}

static bool needs_horizontal_flip(int rotation_value){
  if(rotation_value==180)return true;
  return false;
}
static bool needs_vertical_flip(int rotation_value){
  if(rotation_value==180)return true;
  return false;
}

static bool validate_mjpeg_quality_percent(int value){
  return value<=100 && value>=1;
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
  std::cout<<"Parsing:{"<<videoFormat<<"}\n";
  TmpVideoFormat tmp_video_format{0,0,0};
  const std::regex reg{R"((\d*)x(\d*)\@(\d*))"};
  std::smatch result;
  if (std::regex_search(videoFormat, result, reg)) {
	//std::cout<<"Got regex size:"<<result.size();
	if (result.size() == 4) {
	  std::cout<<"result[0]={"<<result[0].str()<<"}\n";
	  tmp_video_format.width_px=atoi(result[1].str().c_str());
	  tmp_video_format.height_px=atoi(result[2].str().c_str());
	  tmp_video_format.framerate=atoi(result[3].str().c_str());
	  if(validate_video_with(tmp_video_format.width_px) &&
	  	validate_video_height(tmp_video_format.height_px)&& validate_video_fps(tmp_video_format.framerate)){
		std::stringstream log;
		log<<"Final result:{"<<video_format_from_int_values(tmp_video_format.width_px,tmp_video_format.height_px,tmp_video_format.framerate);
		log<<"}\n";
		std::cout<<log.str();
		return tmp_video_format;
	  }
	}
  }
  return std::nullopt;
}


}
#endif //OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
