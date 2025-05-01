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

#include "wifi_hotspot.h"

#include <iostream>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_util_async.h"

static constexpr auto OHD_WIFI_HOTSPOT_CONNECTION_NAME = "ohd_wifi_hotspot";
const std::string blue = "\033[34m";
const std::string reset = "\033[0m";

static std::string get_ohd_wifi_hotspot_connection_nm_filename() {
  return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",
                     OHD_WIFI_HOTSPOT_CONNECTION_NAME);
}

// NOTE: This creates the proper NM connection, but does not start it yet.
static bool create_hotspot_connection_file(const WiFiCard& card,
                                           const bool is_air,
                                           const bool use_5g_channel) {
  // delete any previous connection that might exist. This might fail if no
  // connection of that name exists - aka an error here can be ignored. We
  // re-create it just to be sure, since for example, the wifi card might have
  // been changed during re-boots.
  if (OHDFilesystemUtil::exists(
          get_ohd_wifi_hotspot_connection_nm_filename())) {
    OHDUtil::run_command("nmcli",
                         {"con", "delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  }
  // and create the hotspot one
  OHDUtil::run_command(
      "nmcli",
      {"con add type wifi ifname", card.device_name, "con-name",
       OHD_WIFI_HOTSPOT_CONNECTION_NAME, "autoconnect no",
       fmt::format("ssid {}", is_air ? "openhd_air" : "openhd_ground")});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        " 802-11-wireless.mode ap", "802-11-wireless.band",
                        use_5g_channel ? "a" : "bg", "ipv4.method shared"});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        " wifi-sec.key-mgmt wpa-psk"});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        " wifi-sec.psk \"openhdopenhd\""});
  OHDUtil::run_command("nmcli", {"con modify", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                                 "ipv4.addresses 192.168.3.1/24"});
  return true;
}

bool WifiHotspot::util_delete_nm_file() {
  // cleanup - proper stop of openhd, do not leave any traces behind.
  if (OHDFilesystemUtil::exists(
          get_ohd_wifi_hotspot_connection_nm_filename())) {
    OHDUtil::run_command("nmcli",
                         {"con", "delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
    return true;
  }
  return false;
}

WifiHotspot::WifiHotspot(OHDProfile profile, WiFiCard wifiCard,
                         const openhd::WifiSpace& wifibroadcast_frequency_space)
    : m_profile(std::move(profile)), m_wifi_card(std::move(wifiCard)) {
  m_use_5G_channel = WifiHotspot::get_use_5g_channel(
      m_wifi_card, wifibroadcast_frequency_space);
  m_console = openhd::log::create_or_get("wifi_hs");
  // create the connection (no matter if hotspot is enabled) such that we can
  // just enable / disable it by running connection up / down.
  m_console->debug("begin create hotspot connection");
  create_hotspot_connection_file(m_wifi_card, m_profile.is_air,
                                 m_use_5G_channel);
  m_console->debug("end create hotspot connection");
}

WifiHotspot::~WifiHotspot() { util_delete_nm_file(); }

void WifiHotspot::start() {
  m_console->debug("Starting WIFI hotspot on card {}", m_wifi_card.device_name);
  const auto args =
      std::vector<std::string>{"con", "up", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli", args);
  started = true;
  m_console->info("Wifi hotspot started");
  std::cout << blue << "Started WIFI hotspot on card "
            << m_wifi_card.device_name << reset << std::endl;
}

void WifiHotspot::stop() {
  m_console->debug("Stopping wifi hotspot on card {}", m_wifi_card.device_name);
  if (!started) return;
  const auto args =
      std::vector<std::string>{"con", "down", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli", args);
  m_console->info("Wifi hotspot stopped");
}

void WifiHotspot::start_async() {
  openhd::AsyncHandle::instance().execute_async(
      "WiFi HS", [this]() { WifiHotspot::start(); });
}

void WifiHotspot::stop_async() {
  openhd::AsyncHandle::instance().execute_async(
      "WiFi HS", [this]() { WifiHotspot::stop(); });
}

void WifiHotspot::set_enabled_async(bool enable) {
  if (m_is_enabled == enable) return;
  m_is_enabled = enable;
  if (enable) {
    start_async();
  } else {
    stop_async();
  }
}

bool WifiHotspot::get_use_5g_channel(
    const WiFiCard& wifiCard,
    const openhd::WifiSpace& wifibroadcast_frequency_space) {
  const bool wifibroadcast_uses_5G =
      wifibroadcast_frequency_space == openhd::WifiSpace::G5_8;
  bool should_use_5G = !wifibroadcast_uses_5G;
  if (should_use_5G && !wifiCard.supports_5GHz()) {
    openhd::log::get_default()->debug(
        "openhd needs 5G hotspot but hotspot card only supports 2G,you'l get "
        "really bad interference");
    openhd::log::get_default()->debug("Using 2.4G hotspot");
    should_use_5G = false;
  }
  // Not seen a 5G only card yet
  return should_use_5G;
}

uint16_t WifiHotspot::get_frequency() {
  if (m_use_5G_channel) return 5180;
  return 2412;
}
