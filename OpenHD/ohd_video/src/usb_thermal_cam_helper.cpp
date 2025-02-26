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

#include "usb_thermal_cam_helper.h"

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_async.h"

void openhd::set_infiray_custom_control_zoom_absolute_async(
    int value, int v4l2_device_number) {
  if (!is_valid_infiray_custom_control_zoom_absolute_value(value)) {
    openhd::log::get_default()->debug(
        "set_infiray_custom_control_zoom_absolute_async {} not valid", value);
    return;
  }
  if (v4l2_device_number < 0) v4l2_device_number = 0;
  const auto command =
      fmt::format("v4l2-ctl -d /dev/video{} -c zoom_absolute={}",
                  v4l2_device_number, value);
  openhd::AsyncHandle::instance().execute_command_async("INFIRAY", command);
}
