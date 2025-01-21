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

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_

#include <string>
#include <vector>

namespace openhd {

static constexpr auto RPI_ETHERNET_ONLY = "RPI_ETHERNET_ONLY";

// NOTE: Read the .config - file itself for documentation on what these
// variables do
struct Config {
  // WIFI
  bool WIFI_ENABLE_AUTODETECT = true;
  std::vector<std::string> WIFI_WB_LINK_CARDS{};
  std::string WIFI_WIFI_HOTSPOT_CARD;
  bool WIFI_MONITOR_CARD_EMULATE = false;
  bool WIFI_FORCE_NO_LINK_BUT_HOTSPOT = false;
  bool WIFI_LOCAL_NETWORK_ENABLE = false;
  std::string WIFI_LOCAL_NETWORK_SSID;
  std::string WIFI_LOCAL_NETWORK_PASSWORD;

  // NETWORKING
  std::string NW_ETHERNET_CARD = RPI_ETHERNET_ONLY;
  std::vector<std::string> NW_MANUAL_FORWARDING_IPS;
  bool NW_FORWARD_TO_LOCALHOST_58XX = false;

  // ETHERNET LINK
  std::string GROUND_UNIT_IP = "";
  std::string AIR_UNIT_IP = "";
  int VIDEO_PORT = 5000;
  int TELEMETRY_PORT = 5600;

  // ETHERNET LINK FOR MICROHARD
  bool DISABLE_MICROHARD_DETECTION = false;
  bool FORCE_MICROHARD = false;
  std::string MICROHARD_USERNAME = "admin";
  std::string MICROHARD_PASSWORD = "qwertz1";
  std::string MICROHARD_IP_AIR = "";
  std::string MICROHARD_IP_GROUND = "";
  std::string MICROHARD_IP_RANGE = "";
  int MICROHARD_VIDEO_PORT = 5910;
  int MICROHARD_TELEMETRY_PORT = 5920;
  // GENERAL
  bool GEN_ENABLE_LAST_KNOWN_POSITION = false;
  int GEN_RF_METRICS_LEVEL = 0;
  bool GEN_NO_QOPENHD_AUTOSTART = false;
};

// Otherwise, default location is used
void set_config_file(const std::string& config_file_path);

Config load_config();

void debug_config(const Config& config);
void debug_config();

// Control ethernet via mavlink & Network manager - only on rpi by default,
// otherwise card needs to be manually specified
bool nw_ethernet_card_manual_active(const Config& config);

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
