//
// Created by consti10 on 21.01.23.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_

#include <map>

#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

namespace openhd::telemetry::rpi {

// Do not modify the GPIO in any way (default)
static constexpr int GPIO_LEAVE_UNTOUCHED = 0;
// Set GPIO to high
static constexpr int GPIO_HIGH = 1;
static constexpr int GPIO_LOW = 2;

struct GPIOControlSettings {
  int gpio_2 = GPIO_LEAVE_UNTOUCHED;
  int gpio_26 = GPIO_LEAVE_UNTOUCHED;
};

static const std::string SETTINGS_DIRECTORY =
    std::string(SETTINGS_BASE_PATH) + std::string("telemetry/");

class GPIOControlSettingsHolder
    : public openhd::PersistentSettings<GPIOControlSettings> {
 public:
  GPIOControlSettingsHolder()
      : openhd::PersistentSettings<GPIOControlSettings>(SETTINGS_DIRECTORY) {
    init();
  }

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "rpi_gpio_control.json";
  }
  [[nodiscard]] GPIOControlSettings create_default() const override {
    return GPIOControlSettings{};
  }
  std::optional<GPIOControlSettings> impl_deserialize(
      const std::string& file_as_string) const override;

  std::string imp_serialize(const GPIOControlSettings& data) const override;
};
}  // namespace openhd::telemetry::rpi

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_GPIO_CONTROLL_RASPBERRYPIGPIOCONTROLSETTINGS_H_
