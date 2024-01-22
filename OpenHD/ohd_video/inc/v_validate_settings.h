//
// Created by consti10 on 20.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_

#include <optional>
#include <regex>

#include "openhd_spdlog.h"

// For now, only basic sanity checking on video settings
namespace openhd{

// We used to have some basic "sane user inputs" validation, but there were a couple of issues with that
// 1) Only for USB (UVC) cameras - sometimes, they only work with width, height or (especially) framerate omitted on the gst pipeline
// 2) Thermal cameras as an example can have ultra low resolutions / framerates, e.g. 110 pixels wide
// Therefore, we allow values >=0 for width height and fps, and when they are set to 0, the (gst) argument should be omitted.
// Old stuff left for reference
// max: 3840×2160 (4K)
// min: 320x240
static bool validate_video_with(int video_w){
  //return video_w >= 320 && video_w<=3840;
  return video_w>=0;
}
static bool validate_video_height(int video_h){
  //return video_h >= 240 && video_h<=2160;
  return video_h>=0;
}
// min: 1 fps max: 240 fps
static bool validate_video_fps(int fps){
  //return fps>=1 && fps <= 240;
  return fps>=0;
}

static bool validate_video_width_height_fps(int video_w,int video_h,int fps){
  return validate_video_with(video_w) && validate_video_height(video_h) && validate_video_fps(fps);
}

// 0 or 1 (h264 or h265)
static bool validate_video_codec(int codec){
  return codec==0 || codec==1 ;
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

static bool validate_rpi_brightness(int value){
  return value>=0 && value<=100;
}

static bool validate_rpi_rpicamsrc_iso(int value){
  return value>=0 && value<=3200;
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
  const bool ret=(value>=-1 && value<=2) || value==2130706433;
  if(!ret){
    openhd::log::get_default()->warn("Invalid intra_refresh_type: {}",value);
  }
  return ret;
}

// see gst-rpicamsrc documentation
static bool validate_rpi_rpicamsrc_metering_mode(int value){
  return value>=0 && value<=3;
}

// see libcamera documentation
static bool validate_rpi_libcamera_sharpness_as_int(int value){
    return true;
}
static bool validate_rpi_libcamera_contrast_as_int(int value){
    return true;
}
static bool validate_rpi_libcamera_saturation_as_int(int value){
    return true;
}
static bool validate_rpi_libcamera_ev_value(int value){
    return value>=-10 && value<=10;
}
static bool validate_rpi_libcamera_doenise_index(int value){
  return value>=0 && value<=4;
}
static bool validate_rpi_libcamera_awb_index(int value){
  return value>=0 && value<=7;
}
// (centre, spot, average, custom)
static bool validate_rpi_libcamera_metering_index(int value){
  return value>=0 && value<=2; //metering mode 3 (custom) crashes libcamera
}
// (normal, sport)
static bool validate_rpi_libcamera_exposure_index(int value){
  return value>=0 && value<=1;
}
// cannot really be validated, depends on camera
static bool validate_rpi_libcamera_shutter_microseconds(int value){
  return value>=0 && value<=1000*100;
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
	  //openhd::log::get_default()->debug("result[0]=["+result[0].str()+"]");
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

// remap the openhd libcamera image quality values where we use int
// (but libcamera uses float)
// They are sharpness,contrast,saturation
static float remap_libcamera_openhd_int_to_libcamera_float(int value){
        float tmp=static_cast<float>(value);
        return value/100.0f;
}

}
#endif //OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
