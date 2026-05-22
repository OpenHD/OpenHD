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

#include "AirTelemetrySettings.h"

#include "include_json.hpp"

namespace openhd::telemetry::air {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Settings, fc_uart_connection_type, fc_uart_baudrate, fc_uart_flow_control,
    fc_battery_n_cells, fc_sys_id, openhd_uart_telemetry_connection,
    openhd_uart_telemetry_enabled, openhd_uart_telemetry_baudrate,
    openhd_uart_telemetry_flow_control, openhd_uart_priority_rc,
    openhd_uart_priority_openhd, openhd_uart_priority_fc,
    telemetry_logging_enabled, sbus_out_enabled, sbus_uart_device,
    sbus_update_rate_hz);

std::optional<Settings> SettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  auto parsed = nlohmann::json::parse(file_as_string, nullptr, false);
  if (parsed.is_discarded()) {
    return std::nullopt;
  }
  Settings settings = create_default();
  settings.fc_uart_connection_type =
      parsed.value("fc_uart_connection_type", settings.fc_uart_connection_type);
  settings.fc_uart_baudrate =
      parsed.value("fc_uart_baudrate", settings.fc_uart_baudrate);
  settings.fc_uart_flow_control =
      parsed.value("fc_uart_flow_control", settings.fc_uart_flow_control);
  settings.fc_battery_n_cells =
      parsed.value("fc_battery_n_cells", settings.fc_battery_n_cells);
  settings.fc_sys_id = parsed.value("fc_sys_id", settings.fc_sys_id);
  settings.openhd_uart_telemetry_connection = parsed.value(
      "openhd_uart_telemetry_connection",
      settings.openhd_uart_telemetry_connection);
  settings.openhd_uart_telemetry_enabled = parsed.value(
      "openhd_uart_telemetry_enabled",
      settings.openhd_uart_telemetry_enabled);
  settings.openhd_uart_telemetry_baudrate = parsed.value(
      "openhd_uart_telemetry_baudrate",
      settings.openhd_uart_telemetry_baudrate);
  settings.openhd_uart_telemetry_flow_control = parsed.value(
      "openhd_uart_telemetry_flow_control",
      settings.openhd_uart_telemetry_flow_control);
  settings.openhd_uart_priority_rc =
      parsed.value("openhd_uart_priority_rc", settings.openhd_uart_priority_rc);
  settings.openhd_uart_priority_openhd = parsed.value(
      "openhd_uart_priority_openhd", settings.openhd_uart_priority_openhd);
  settings.openhd_uart_priority_fc =
      parsed.value("openhd_uart_priority_fc", settings.openhd_uart_priority_fc);
  settings.telemetry_logging_enabled = parsed.value(
      "telemetry_logging_enabled", settings.telemetry_logging_enabled);
  settings.sbus_out_enabled =
      parsed.value("sbus_out_enabled", settings.sbus_out_enabled);
  settings.sbus_uart_device =
      parsed.value("sbus_uart_device", settings.sbus_uart_device);
  settings.sbus_update_rate_hz =
      parsed.value("sbus_update_rate_hz", settings.sbus_update_rate_hz);
  return settings;
}

std::string SettingsHolder::imp_serialize(
    const openhd::telemetry::air::Settings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

};  // namespace openhd::telemetry::air
