/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "validate_settings.h"

#include <regex>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

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

std::string openhd::video_format_from_int_values(int width, int height,
                                                 int framerate) {
  std::stringstream ss;
  ss << width << "x" << height << "@" << framerate;
  return ss.str();
}
