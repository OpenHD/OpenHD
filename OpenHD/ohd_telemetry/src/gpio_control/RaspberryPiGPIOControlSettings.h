//
// Created by consti10 on 21.01.23.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_

#include <map>

#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

namespace openhd::telemetry::rpi {

// Do not modify the GPIO in any way (default)
static constexpr int GPIO_LEAVE_UNTOUCHED=0;
// Set GPIO to high
static constexpr int GPIO_HIGH=1;
static constexpr int GPIO_LOW=2;

struct GPIOControlSettings {
  int gpio_2 = GPIO_LEAVE_UNTOUCHED;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GPIOControlSettings,gpio_2);

static const std::string SETTINGS_DIRECTORY =
    std::string(SETTINGS_BASE_PATH) + std::string("telemetry/");

class GPIOControlSettingsHolder
    : public openhd::PersistentJsonSettings<GPIOControlSettings> {
 public:
  GPIOControlSettingsHolder()
      : openhd::PersistentJsonSettings< GPIOControlSettings>(SETTINGS_DIRECTORY) {
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "rpi_gpio_control.json";
  }
  [[nodiscard]] GPIOControlSettings create_default() const override {
    return GPIOControlSettings{};
  }
};
}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_
