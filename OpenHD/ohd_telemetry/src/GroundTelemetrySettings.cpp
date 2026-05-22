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

#include "GroundTelemetrySettings.h"

#include "include_json.hpp"

namespace openhd::telemetry::ground {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Settings, enable_rc_over_joystick, rc_over_joystick_update_rate_hz,
    rc_channel_mapping, fc_sys_id, gnd_uart_connection_type, gnd_uart_baudrate,
    gnd_uart_flow_control, openhd_uart_telemetry_connection,
    openhd_uart_telemetry_enabled, openhd_uart_telemetry_baudrate,
    openhd_uart_telemetry_flow_control, openhd_uart_priority_rc,
    openhd_uart_priority_openhd, openhd_uart_priority_fc,
    telemetry_logging_enabled);

std::optional<Settings>
openhd::telemetry::ground::SettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  auto parsed = nlohmann::json::parse(file_as_string, nullptr, false);
  if (parsed.is_discarded()) {
    return std::nullopt;
  }
  Settings settings = create_default();
  settings.enable_rc_over_joystick =
      parsed.value("enable_rc_over_joystick", settings.enable_rc_over_joystick);
  settings.rc_over_joystick_update_rate_hz = parsed.value(
      "rc_over_joystick_update_rate_hz",
      settings.rc_over_joystick_update_rate_hz);
  settings.rc_channel_mapping =
      parsed.value("rc_channel_mapping", settings.rc_channel_mapping);
  settings.fc_sys_id = parsed.value("fc_sys_id", settings.fc_sys_id);
  settings.gnd_uart_connection_type =
      parsed.value("gnd_uart_connection_type", settings.gnd_uart_connection_type);
  settings.gnd_uart_baudrate =
      parsed.value("gnd_uart_baudrate", settings.gnd_uart_baudrate);
  settings.gnd_uart_flow_control =
      parsed.value("gnd_uart_flow_control", settings.gnd_uart_flow_control);
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
  return settings;
}

std::string openhd::telemetry::ground::SettingsHolder::imp_serialize(
    const openhd::telemetry::ground::Settings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}
};  // namespace openhd::telemetry::ground
