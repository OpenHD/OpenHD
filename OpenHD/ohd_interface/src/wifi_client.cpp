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
#include <vector>

#include "openhd_config.h"
#include "openhd_util_filesystem.h"
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

static constexpr auto OHD_WIFI_CLIENT_CONNECTION_NAME = "ohd_wifi_client";

static std::string get_client_connection_nm_filename() {
  return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",
                     OHD_WIFI_CLIENT_CONNECTION_NAME);
}

static void delete_existing_client_file() {
  if (OHDFilesystemUtil::exists(get_client_connection_nm_filename())) {
    OHDUtil::run_command("nmcli", {"con", "delete",
                                   OHD_WIFI_CLIENT_CONNECTION_NAME});
  }
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

bool WiFiClient::connect(const std::string& interface_name,
                         const std::string& ssid, const std::string& password,
                         std::shared_ptr<spdlog::logger> console) {
  if (interface_name.empty() || ssid.empty() || password.empty()) {
    if (console) {
      console->warn("Cannot start wifi client, missing interface or credentials"
                    " (iface:{}, ssid length:{}, pw length:{})",
                    interface_name, ssid.length(), password.length());
    }
    return false;
  }
  delete_existing_client_file();
  std::vector<std::string> args{
      "device", "wifi", "connect", fmt::format("\"{}\"", ssid), "password",
      fmt::format("\"{}\"", password), "ifname", interface_name, "name",
      OHD_WIFI_CLIENT_CONNECTION_NAME};
  const auto result = OHDUtil::run_command("nmcli", args, true);
  if (console) {
    if (result == 0) {
      console->info("Connecting {} to wifi network {}", interface_name, ssid);
    } else {
      console->warn("Failed to connect {} to wifi network {}", interface_name,
                    ssid);
    }
  }
  return result == 0;
}

void WiFiClient::disconnect(std::shared_ptr<spdlog::logger> console) {
  const auto result =
      OHDUtil::run_command("nmcli", {"con", "down",
                                     OHD_WIFI_CLIENT_CONNECTION_NAME});
  delete_existing_client_file();
  if (console) {
    if (result == 0) {
      console->info("Disconnected wifi client connection {}",
                    OHD_WIFI_CLIENT_CONNECTION_NAME);
    } else {
      console->debug("No active wifi client connection {} to disconnect",
                     OHD_WIFI_CLIENT_CONNECTION_NAME);
    }
  }
}
