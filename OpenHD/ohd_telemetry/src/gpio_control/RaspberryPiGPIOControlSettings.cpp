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

#include "RaspberryPiGPIOControlSettings.h"

#include "include_json.hpp"

namespace openhd::telemetry::rpi {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GPIOControlSettings, gpio_2);

std::optional<GPIOControlSettings> GPIOControlSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<GPIOControlSettings>(file_as_string);
}

std::string GPIOControlSettingsHolder::imp_serialize(
    const GPIOControlSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}
};  // namespace openhd::telemetry::rpi