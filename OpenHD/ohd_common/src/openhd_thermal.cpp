//
// Created by consti10 on 21.04.24.
//

#include "openhd_thermal.h"

#include "openhd_util.h"
#include "openhd_util_filesystem.h"

int openhd::x20_read_rtl8812au_thermal_sensor_degree() {
  auto temp_file_opt = OHDFilesystemUtil::opt_read_file(
      "/sys/class/hwmon/hwmon0/temp1_input", false);
  if (!temp_file_opt.has_value()) {
    return 0;
  }
  auto temp = OHDUtil::string_to_int(temp_file_opt.value());
  if (!temp.has_value()) return 0;
  return temp.value() / 1000;
}
