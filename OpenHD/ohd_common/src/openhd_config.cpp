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
 * Â© OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "openhd_config.h"

#include <cctype>
#include <iostream>
#include <optional>

#include "openhd_sock.h"
#include "openhd_util.h"

namespace {
std::optional<bool> g_wifi_monitor_emulate_override;

std::vector<std::string> split_list(std::string value) {
  if (value.empty()) {
    return {};
  }
  for (auto& ch : value) {
    if (std::isspace(static_cast<unsigned char>(ch)) || ch == ';') {
      ch = ',';
    }
  }
  auto parts = OHDUtil::split_into_substrings(value, ',');
  std::vector<std::string> out;
  out.reserve(parts.size());
  for (auto& part : parts) {
    OHDUtil::trim(part);
    if (!part.empty()) {
      out.push_back(part);
    }
  }
  return out;
}

}  // namespace

static openhd::Config load_or_default() {
  openhd::Config ret{};
  const auto sysutil_settings = openhd::request_sysutil_settings();
  if (!sysutil_settings.has_value()) {
    std::cerr << "WARN: sysutils settings unavailable, using defaults"
              << std::endl;
    return ret;
  }

  const auto& settings = sysutil_settings.value();
  ret.WIFI_ENABLE_AUTODETECT = settings.wifi_enable_autodetect;
  ret.WIFI_WB_LINK_CARDS = split_list(settings.wifi_wb_link_cards);
  ret.WIFI_WIFI_HOTSPOT_CARD = settings.wifi_hotspot_card;
  ret.WIFI_MONITOR_CARD_EMULATE = settings.wifi_monitor_card_emulate;
  ret.WIFI_FORCE_NO_LINK_BUT_HOTSPOT = settings.wifi_force_no_link_but_hotspot;
  ret.WIFI_LOCAL_NETWORK_ENABLE = settings.wifi_local_network_enable;
  ret.WIFI_LOCAL_NETWORK_SSID = settings.wifi_local_network_ssid;
  ret.WIFI_LOCAL_NETWORK_PASSWORD = settings.wifi_local_network_password;
  ret.NW_ETHERNET_CARD = settings.nw_ethernet_card;
  ret.NW_MANUAL_FORWARDING_IPS = split_list(settings.nw_manual_forwarding_ips);
  ret.NW_FORWARD_TO_LOCALHOST_58XX = settings.nw_forward_to_localhost_58xx;
  ret.GROUND_UNIT_IP = settings.ground_unit_ip;
  ret.AIR_UNIT_IP = settings.air_unit_ip;
  ret.VIDEO_PORT = settings.video_port;
  ret.TELEMETRY_PORT = settings.telemetry_port;
  ret.DISABLE_MICROHARD_DETECTION = settings.disable_microhard_detection;
  ret.FORCE_MICROHARD = settings.force_microhard;
  ret.MICROHARD_USERNAME = settings.microhard_username;
  ret.MICROHARD_PASSWORD = settings.microhard_password;
  ret.MICROHARD_IP_AIR = settings.microhard_ip_air;
  ret.MICROHARD_IP_GROUND = settings.microhard_ip_ground;
  ret.MICROHARD_IP_RANGE = settings.microhard_ip_range;
  ret.MICROHARD_VIDEO_PORT = settings.microhard_video_port;
  ret.MICROHARD_TELEMETRY_PORT = settings.microhard_telemetry_port;
  ret.GEN_ENABLE_LAST_KNOWN_POSITION = settings.gen_enable_last_known_position;
  ret.GEN_RF_METRICS_LEVEL = settings.gen_rf_metrics_level;
  if (g_wifi_monitor_emulate_override.has_value()) {
    ret.WIFI_MONITOR_CARD_EMULATE = g_wifi_monitor_emulate_override.value();
  }
  return ret;
}

openhd::Config openhd::load_config() {
  static openhd::Config config = load_or_default();
  return config;
}

void openhd::set_wifi_monitor_card_emulate_override(
    std::optional<bool> value) {
  g_wifi_monitor_emulate_override = value;
}

void openhd::debug_config(const openhd::Config& config) {
  std::cout << "DEBUG: WIFI_ENABLE_AUTODETECT: "
            << config.WIFI_ENABLE_AUTODETECT << std::endl;
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
