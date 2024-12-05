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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROL_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROL_H_

#include <memory>

#include "RaspberryPiGPIOControlSettings.h"
#include "openhd_settings_imp.h"

namespace openhd::telemetry::rpi {

// This class exposes the following feature on rpi:
// Control GPIO pins (set them to low / high to - for example - control a
// landing gear) via the openhd mavlink settings (mavlink extended parameters'
// protocol)
class GPIOControl {
 public:
  GPIOControl();
  std::vector<openhd::Setting> get_all_settings();

 private:
  std::unique_ptr<openhd::telemetry::rpi::GPIOControlSettingsHolder> m_settings;
};

}  // namespace openhd::telemetry::rpi

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROL_H_
