//
// Created by consti10 on 19.09.23.
//
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