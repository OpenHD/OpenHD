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

#include "wifi_client.h"

#include <openhd_spdlog.h>

#include "openhd_config.h"
#include "wifi_card.h"
#include "wifi_hotspot.h"

static std::string create_command_wifi_client(const std::string ssid,
                                              const std::string pw) {
  return fmt::format("sudo nmcli dev wifi connect \"{}\" password \"{}\"", ssid,
                     pw);
}

static std::shared_ptr<spdlog::logger> get_console() {
  return openhd::log::create_or_get("WiFiClient");
}

bool WiFiClient::create_if_enabled() {
  const auto config = openhd::load_config();
  if (!config.WIFI_LOCAL_NETWORK_ENABLE) {
    return false;
  }
  const auto command = create_command_wifi_client(
      config.WIFI_LOCAL_NETWORK_SSID, config.WIFI_LOCAL_NETWORK_PASSWORD);
  OHDUtil::run_command(command, {}, true);
  return true;
}
