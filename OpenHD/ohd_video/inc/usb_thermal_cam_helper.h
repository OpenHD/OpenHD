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

#ifndef OPENHD_USB_THERMAL_CAM_HELPER_H
#define OPENHD_USB_THERMAL_CAM_HELPER_H

namespace openhd {

static bool is_valid_infiray_custom_control_zoom_absolute_value(int value) {
  return value >= 0 && value <= 10000000;
  // return value >=32768 && value<=34826;
  // return value>= 34816 &&  value<=34826;
}

// Apply the setting and hope it works ...
// Most of the time the device number is /dev/video0
void set_infiray_custom_control_zoom_absolute_async(int value,
                                                    int v4l2_device_number);

}  // namespace openhd

#endif  // OPENHD_USB_THERMAL_CAM_HELPER_H
