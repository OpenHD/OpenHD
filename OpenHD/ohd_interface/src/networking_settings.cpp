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

#include "networking_settings.h"

#include "include_json.hpp"

static void to_json(nlohmann::json& j, const NetworkingSettings& settings) {
  j = nlohmann::json{
      {"wifi_operating_mode", settings.wifi_operating_mode},
      {"wifi_hotspot_mode", settings.wifi_hotspot_mode},
      {"wifi_hotspot_interface_override",
       settings.wifi_hotspot_interface_override},
      {"wifi_hotspot_ssid", settings.wifi_hotspot_ssid},
      {"wifi_hotspot_password", settings.wifi_hotspot_password},
      {"wifi_client_interface", settings.wifi_client_interface},
      {"wifi_client_ssid", settings.wifi_client_ssid},
      {"wifi_client_password", settings.wifi_client_password},
      {"ethernet_operating_mode", settings.ethernet_operating_mode},
      {"enterprise_multi_link", settings.enterprise_multi_link}};
}

static void from_json(const nlohmann::json& j, NetworkingSettings& settings) {
  settings.wifi_operating_mode =
      j.value("wifi_operating_mode", settings.wifi_operating_mode);
  settings.wifi_hotspot_mode =
      j.value("wifi_hotspot_mode", settings.wifi_hotspot_mode);
  settings.wifi_hotspot_interface_override =
      j.value("wifi_hotspot_interface_override",
              settings.wifi_hotspot_interface_override);
  settings.wifi_hotspot_ssid =
      j.value("wifi_hotspot_ssid", settings.wifi_hotspot_ssid);
  settings.wifi_hotspot_password =
      j.value("wifi_hotspot_password", settings.wifi_hotspot_password);
  settings.wifi_client_interface =
      j.value("wifi_client_interface", settings.wifi_client_interface);
  settings.wifi_client_ssid =
      j.value("wifi_client_ssid", settings.wifi_client_ssid);
  settings.wifi_client_password =
      j.value("wifi_client_password", settings.wifi_client_password);
  settings.ethernet_operating_mode =
      j.value("ethernet_operating_mode", settings.ethernet_operating_mode);
  settings.enterprise_multi_link =
      j.value("enterprise_multi_link", settings.enterprise_multi_link);
}

std::optional<NetworkingSettings> NetworkingSettingsHolder::impl_deserialize(
    const std::string& file_as_string) const {
  return openhd_json_parse<NetworkingSettings>(file_as_string);
}

std::string NetworkingSettingsHolder::imp_serialize(
    const NetworkingSettings& data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}
