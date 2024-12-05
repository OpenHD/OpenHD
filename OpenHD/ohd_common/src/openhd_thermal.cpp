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
