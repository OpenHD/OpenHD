//
// Created by consti10 on 21.01.23.
//

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
