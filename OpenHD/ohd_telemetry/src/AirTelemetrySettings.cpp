//
// Created by consti10 on 19.09.23.
//
#include "AirTelemetrySettings.h"

#include "include_json.hpp"

namespace openhd::telemetry::air {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, fc_uart_connection_type,
                                   fc_uart_baudrate, fc_uart_flow_control,
                                   fc_battery_n_cells);

std::optional<Settings> SettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<Settings>(file_as_string);
}

std::string SettingsHolder::imp_serialize(
    const openhd::telemetry::air::Settings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

};  // namespace openhd::telemetry::air
