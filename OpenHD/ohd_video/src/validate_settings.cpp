//
// Created by consti10 on 30.01.24.
//

#include "validate_settings.h"

#include <regex>

#include "openhd_spdlog.h"

bool openhd::validate_bitrate_mbits(int bitrate_mbits) {
  const bool ret = bitrate_mbits >= 1 && bitrate_mbits <= 50;
  if (!ret) {
    openhd::log::get_default()->warn("Invalid bitrate_mbits: {}",
                                     bitrate_mbits);
  }
  return ret;
}

bool openhd::validate_camera_rotation(int value) {
  const bool ret = value == 0 || value == 90 || value == 180 || value == 270;
  if (!ret) {
    openhd::log::get_default()->warn("Invalid camera_rotation: {}", value);
  }
  return ret;
}

bool openhd::validate_rpi_keyframe_interval(int value) {
  const bool ret = value >= -1 && value < 2147483647;
  if (!ret) {
    openhd::log::get_default()->warn("Invalid rpi_keyframe_interval: {}",
                                     value);
  }
  return ret;
}

bool openhd::validate_rpi_intra_refresh_type(int value) {
  const bool ret = (value >= -1 && value <= 2) || value == 2130706433;
  if (!ret) {
    openhd::log::get_default()->warn("Invalid intra_refresh_type: {}", value);
  }
  return ret;
}

std::optional<openhd::TmpVideoFormat> openhd::parse_video_format(
    const std::string &videoFormat) {
  if (videoFormat.size() <= 5) {
    return std::nullopt;
  }
  openhd::log::get_default()->debug("Parsing:[" + videoFormat + "]");
  TmpVideoFormat tmp_video_format{0, 0, 0};
  const std::regex reg{R"((\d*)x(\d*)\@(\d*))"};
  std::smatch result;
  if (std::regex_search(videoFormat, result, reg)) {
    if (result.size() == 4) {
      // openhd::log::get_default()->debug("result[0]=["+result[0].str()+"]");
      tmp_video_format.width_px = atoi(result[1].str().c_str());
      tmp_video_format.height_px = atoi(result[2].str().c_str());
      tmp_video_format.framerate = atoi(result[3].str().c_str());
      if (validate_video_with(tmp_video_format.width_px) &&
          validate_video_height(tmp_video_format.height_px) &&
          validate_video_fps(tmp_video_format.framerate)) {
        std::stringstream log;
        log << "Final result:{"
            << video_format_from_int_values(tmp_video_format.width_px,
                                            tmp_video_format.height_px,
                                            tmp_video_format.framerate);
        log << "}";
        openhd::log::get_default()->debug(log.str());
        return tmp_video_format;
      }
    }
  }
  return std::nullopt;
}

std::string openhd::video_format_from_int_values(int width, int height,
                                                 int framerate) {
  std::stringstream ss;
  ss << width << "x" << height << "@" << framerate;
  return ss.str();
}
