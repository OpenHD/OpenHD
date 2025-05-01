/*******************************************************************************
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

#include "openhd_config.h"

#include "../lib/ini/ini.hpp"
#include "config_paths.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static std::string CONFIG_FILE_PATH =
    std::string(getConfigBasePath()) + "hardware.config";

void openhd::set_config_file(const std::string& config_file_path) {
  std::cout << "DEBUG: Using custom config file path [" << config_file_path
            << "]" << std::endl;
  CONFIG_FILE_PATH = config_file_path;
}

static openhd::Config load_or_default() {
  try {
    openhd::Config ret{};
    if (OHDFilesystemUtil::exists("/usr/local/share/openhd/debug.txt")) {
      if (!OHDFilesystemUtil::exists(CONFIG_FILE_PATH)) {
        std::cerr << "WARN: No config file [" << CONFIG_FILE_PATH << "] used!"
                  << std::endl;
        return ret;
      } else {
        std::cout << "WARN: Advanced config file [" << CONFIG_FILE_PATH
                  << "] used!" << std::endl;
      }
    }
    inih::INIReader r{CONFIG_FILE_PATH};
    if (OHDFilesystemUtil::exists("/usr/local/share/openhd/debug.txt")) {
      // Parse WiFi configuration
      ret.WIFI_ENABLE_AUTODETECT =
          r.Get<bool>("wifi", "WIFI_ENABLE_AUTODETECT", false);
      ret.WIFI_WB_LINK_CARDS =
          r.GetVector<std::string>("wifi", "WIFI_WB_LINK_CARDS");
      ret.WIFI_WIFI_HOTSPOT_CARD =
          r.Get<std::string>("wifi", "WIFI_WIFI_HOTSPOT_CARD", "");
      ret.WIFI_MONITOR_CARD_EMULATE =
          r.Get<bool>("wifi", "WIFI_MONITOR_CARD_EMULATE", false);
      ret.WIFI_FORCE_NO_LINK_BUT_HOTSPOT =
          r.Get<bool>("wifi", "WIFI_FORCE_NO_LINK_BUT_HOTSPOT", false);
      ret.WIFI_LOCAL_NETWORK_ENABLE =
          r.Get<bool>("wifi", "WIFI_LOCAL_NETWORK_ENABLE", false);
      ret.WIFI_LOCAL_NETWORK_SSID =
          r.Get<std::string>("wifi", "WIFI_LOCAL_NETWORK_SSID", "");
      ret.WIFI_LOCAL_NETWORK_PASSWORD =
          r.Get<std::string>("wifi", "WIFI_LOCAL_NETWORK_PASSWORD", "");

      // Parse Network configuration
      ret.NW_ETHERNET_CARD =
          r.Get<std::string>("network", "NW_ETHERNET_CARD", "");
      ret.NW_MANUAL_FORWARDING_IPS =
          r.GetVector<std::string>("network", "NW_MANUAL_FORWARDING_IPS");
      ret.NW_FORWARD_TO_LOCALHOST_58XX =
          r.Get<bool>("network", "NW_FORWARD_TO_LOCALHOST_58XX", false);

      // Parse Ethernet link configuration
      std::cout << "WARN: Parsing Ethernet link configuration" << std::endl;
      ret.GROUND_UNIT_IP = r.Get<std::string>("ethernet", "GROUND_UNIT_IP", "");
      std::cout << "DEBUG: GROUND_UNIT_IP: " << ret.GROUND_UNIT_IP << std::endl;
      ret.AIR_UNIT_IP = r.Get<std::string>("ethernet", "AIR_UNIT_IP", "");
      std::cout << "DEBUG: AIR_UNIT_IP: " << ret.AIR_UNIT_IP << std::endl;
      ret.VIDEO_PORT = r.Get<int>("ethernet", "VIDEO_PORT", 5000);
      std::cout << "DEBUG: VIDEO_PORT: " << ret.VIDEO_PORT << std::endl;
      ret.TELEMETRY_PORT = r.Get<int>("ethernet", "TELEMETRY_PORT", 5600);
      std::cout << "DEBUG: TELEMETRY_PORT: " << ret.TELEMETRY_PORT << std::endl;

      // Parse Ethernet link Microhard configuration
      std::cout << "WARN: Parsing Ethernet link Microhard configuration"
                << std::endl;
      ret.DISABLE_MICROHARD_DETECTION =
          r.Get<bool>("microhard", "DISABLE_MICROHARD_DETECTION", false);
      std::cout << "DEBUG: DISABLE_MICROHARD_DETECTION: "
                << ret.DISABLE_MICROHARD_DETECTION << std::endl;
      ret.FORCE_MICROHARD = r.Get<bool>("microhard", "FORCE_MICROHARD", false);
      std::cout << "DEBUG: FORCE_MICROHARD: " << ret.FORCE_MICROHARD
                << std::endl;
      ret.MICROHARD_USERNAME =
          r.Get<std::string>("microhard", "MICROHARD_USERNAME", "admin");
      std::cout << "DEBUG: MICROHARD_USERNAME: " << ret.MICROHARD_USERNAME
                << std::endl;
      ret.MICROHARD_PASSWORD =
          r.Get<std::string>("microhard", "MICROHARD_PASSWORD", "qwertz1");
      std::cout << "DEBUG: MICROHARD_PASSWORD: " << ret.MICROHARD_PASSWORD
                << std::endl;
      ret.MICROHARD_IP_AIR =
          r.Get<std::string>("microhard", "MICROHARD_IP_AIR", "");
      std::cout << "DEBUG: MICROHARD_IP_AIR: " << ret.MICROHARD_IP_AIR
                << std::endl;
      ret.MICROHARD_IP_GROUND =
          r.Get<std::string>("microhard", "MICROHARD_IP_GROUND", "");
      std::cout << "DEBUG: MICROHARD_IP_GROUND: " << ret.MICROHARD_IP_GROUND
                << std::endl;
      ret.MICROHARD_IP_RANGE =
          r.Get<std::string>("microhard", "MICROHARD_IP_RANGE", "192.168.168");
      std::cout << "DEBUG: MICROHARD_IP_RANGE: " << ret.MICROHARD_IP_RANGE
                << std::endl;
      ret.MICROHARD_VIDEO_PORT =
          r.Get<int>("microhard", "MICROHARD_VIDEO_PORT", 5910);
      std::cout << "DEBUG: MICROHARD_VIDEO_PORT: " << ret.MICROHARD_VIDEO_PORT
                << std::endl;
      ret.TELEMETRY_PORT =
          r.Get<int>("microhard", "MICROHARD_TELEMETRY_PORT", 5920);
      std::cout << "DEBUG: MICROHARD_TELEMETRY_PORT: " << ret.TELEMETRY_PORT
                << std::endl;
    }
    // Parse Generic configuration
    ret.GEN_ENABLE_LAST_KNOWN_POSITION =
        r.Get<bool>("generic", "GEN_ENABLE_LAST_KNOWN_POSITION", false);
    ret.GEN_RF_METRICS_LEVEL = r.Get<int>("generic", "GEN_RF_METRICS_LEVEL", 0);
    ret.GEN_NO_QOPENHD_AUTOSTART =
        r.Get<bool>("generic", "GEN_NO_QOPENHD_AUTOSTART", false);
    return ret;
  } catch (std::exception& exception) {
    std::cerr << "ERROR: Ill-formatted config file: " << exception.what()
              << std::endl;
  }
  return {};
}

openhd::Config openhd::load_config() {
  static openhd::Config config = load_or_default();
  return config;
}

void openhd::debug_config(const openhd::Config& config) {
  std::cout << "DEBUG: WIFI_ENABLE_AUTODETECT: "
            << config.WIFI_ENABLE_AUTODETECT << std::endl;
  // Add more fields as needed for debugging...
}

void openhd::debug_config() {
  auto config = load_config();
  debug_config(config);
}

bool openhd::nw_ethernet_card_manual_active(const openhd::Config& config) {
  if (OHDUtil::contains(config.NW_ETHERNET_CARD, RPI_ETHERNET_ONLY)) {
    return false;
  }
  return true;
}
