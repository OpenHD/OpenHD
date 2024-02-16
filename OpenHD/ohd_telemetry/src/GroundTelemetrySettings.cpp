//
// Created by consti10 on 19.09.23.
//
#include "GroundTelemetrySettings.h"

#include "include_json.hpp"

namespace openhd::telemetry::ground {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, enable_rc_over_joystick,
                                   rc_over_joystick_update_rate_hz,
                                   rc_channel_mapping, gnd_uart_connection_type,
                                   gnd_uart_baudrate);

std::optional<Settings>
openhd::telemetry::ground::SettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<Settings>(file_as_string);
}

std::string openhd::telemetry::ground::SettingsHolder::imp_serialize(
    const openhd::telemetry::ground::Settings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}
};  // namespace openhd::telemetry::ground
