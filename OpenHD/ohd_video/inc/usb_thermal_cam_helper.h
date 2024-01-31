//
// Created by consti10 on 30.01.24.
//

#ifndef OPENHD_USB_THERMAL_CAM_HELPER_H
#define OPENHD_USB_THERMAL_CAM_HELPER_H

namespace openhd {

static bool is_valid_infiray_custom_control_zoom_absolute_value(int value) {
  return value >= 0 && value <= 10000000;
  // return value >=32768 && value<=34826;
  // return value>= 34816 &&  value<=34826;
}

// Apply the setting and hope it works ...
void set_infiray_custom_control_zoom_absolute_async(int value);

}  // namespace openhd

#endif  // OPENHD_USB_THERMAL_CAM_HELPER_H
