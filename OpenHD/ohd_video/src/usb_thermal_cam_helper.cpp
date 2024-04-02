//
// Created by consti10 on 30.01.24.
//

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
