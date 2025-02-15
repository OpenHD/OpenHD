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
  
  //NMCLI
  // if (OHDFilesystemUtil::exists(
  //         get_ohd_wifi_hotspot_connection_nm_filename())) {
  //   OHDUtil::run_command("nmcli",
  //                        {"con", "delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  // }
  // // and create the hotspot one
  // OHDUtil::run_command(
  //     "nmcli",
  //     {"con add type wifi ifname", card.device_name, "con-name",
  //      OHD_WIFI_HOTSPOT_CONNECTION_NAME, "autoconnect no",
  //      fmt::format("ssid {}", is_air ? "openhd_air" : "openhd_ground")});
  // OHDUtil::run_command("nmcli",
  //                      {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
  //                       " 802-11-wireless.mode ap", "802-11-wireless.band",
  //                       use_5g_channel ? "a" : "bg", "ipv4.method shared"});
  // OHDUtil::run_command("nmcli",
  //                      {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
  //                       " wifi-sec.key-mgmt wpa-psk"});
  // OHDUtil::run_command("nmcli",
  //                      {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
  //                       " wifi-sec.psk \"openhdopenhd\""});
  // OHDUtil::run_command("nmcli", {"con modify", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
  //                                "ipv4.addresses 192.168.3.1/24"});
  //CONNMAN
  OHDUtil::run_command("connmanctl", {"enable wifi"});
  OHDUtil::run_command("connmanctl", {"tether wifi on"});
  OHDUtil::run_command("connmanctl", {"tether wifi set ssid", is_air ? "openhd_air" : "openhd_ground"});
  OHDUtil::run_command("connmanctl", {"tether wifi set passphrase", "\"openhdopenhd\""});

  // Assign a static IP
  OHDUtil::run_command("ip", {"addr add 192.168.3.1/24 dev wlan0"});

  // Restart ConnMan to apply changes
  OHDUtil::run_command("/etc/init.d/S45connman", {"restart"});
  m_console->warn("connection created");

  return true;
}

bool WifiHotspot::util_delete_nm_file() {
  // // cleanup - proper stop of openhd, do not leave any traces behind.
  // if (OHDFilesystemUtil::exists(
  //         get_ohd_wifi_hotspot_connection_nm_filename())) {
  //   OHDUtil::run_command("nmcli",
  //                        {"con", "delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  //   return true;
  // }
  // return false;

  //CONMAN
  // Check if tethering is enabled (if so, disable it)
  std::string output = OHDUtil::run_command("connmanctl", {"state"});
  if (output.find("Tethering") != std::string::npos) {
      OHDUtil::run_command("connmanctl", {"tether wifi off"});
  }

  // Remove manually assigned static IP address
  OHDUtil::run_command("ip", {"addr flush dev wlan0"});

  // Restart ConnMan manually (since no systemctl)
  OHDUtil::run_command("killall", {"connmand"});
  OHDUtil::run_command("connmand", {"-n", "&"});

  return true;
}

WifiHotspot::WifiHotspot(OHDProfile profile, WiFiCard wifiCard,
                         const openhd::WifiSpace& wifibroadcast_frequency_space)
    : m_profile(std::move(profile)), m_wifi_card(std::move(wifiCard)) {
  m_use_5G_channel = WifiHotspot::get_use_5g_channel(
      m_wifi_card, wifibroadcast_frequency_space);
  m_console = openhd::log::create_or_get("wifi_hs");
  // create the connection (no matter if hotspot is enabled) such that we can
  // just enable / disable it by running connection up / down.
  m_console->warn("begin create hotspot connection");
  create_hotspot_connection_file(m_wifi_card, m_profile.is_air,
                                 m_use_5G_channel);
  m_console->warn("end create hotspot connection");
}

WifiHotspot::~WifiHotspot() { util_delete_nm_file(); }

void WifiHotspot::start() {
  // m_console->warn("Starting WIFI hotspot on card {}", m_wifi_card.device_name);
  // const auto args =
  //     std::vector<std::string>{"con", "up", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  // OHDUtil::run_command("nmcli", args);
  // started = true;
  // m_console->info("Wifi hotspot started");
  // std::cout << blue << "Started WIFI hotspot on card "
  //           << m_wifi_card.device_name << reset << std::endl;

  //CONNMAN
  m_console->warn("Starting WIFI hotspot on card {}", m_wifi_card.device_name);

    // Enable Wi-Fi if not already enabled
    OHDUtil::run_command("connmanctl", {"enable wifi"});

    // Start Wi-Fi tethering (hotspot)
    OHDUtil::run_command("connmanctl", {"tether wifi on"});

    // Set the SSID dynamically
    OHDUtil::run_command("connmanctl", 
                         {"tether wifi set ssid", is_air ? "openhd_air" : "openhd_ground"});

    // Set WPA2 Passphrase
    OHDUtil::run_command("connmanctl", {"tether wifi set passphrase", "\"openhdopenhd\""});

    // Assign a Static IP manually
    OHDUtil::run_command("ip", {"addr flush dev wlan0"});
    OHDUtil::run_command("ip", {"addr add 192.168.3.1/24 dev wlan0"});
    OHDUtil::run_command("ip", {"link set wlan0 up"});

    // Ensure ConnMan is restarted without systemd/systemctl
    OHDUtil::run_command("killall", {"connmand"});
    OHDUtil::run_command("connmand", {"-n", "&"});

    started = true;
    m_console->info("Wifi hotspot started");
    std::cout << blue << "Started WIFI hotspot on card "
              << m_wifi_card.device_name << reset << std::endl;
}

void WifiHotspot::stop() {
  // m_console->warn("Stopping wifi hotspot on card {}", m_wifi_card.device_name);
  // if (!started) return;
  // const auto args =
  //     std::vector<std::string>{"con", "down", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  // OHDUtil::run_command("nmcli", args);
  // m_console->info("Wifi hotspot stopped");

  //CONNMAN
  m_console->warn("Stopping wifi hotspot on card {}", m_wifi_card.device_name);
    
  if (!started) return;

  // Disable Wi-Fi tethering (hotspot mode)
  OHDUtil::run_command("connmanctl", {"tether wifi off"});

  // Remove manually assigned static IP
  OHDUtil::run_command("ip", {"addr flush dev wlan0"});

  // Restart ConnMan manually (without systemd)
  OHDUtil::run_command("killall", {"connmand"});
  OHDUtil::run_command("connmand", {"-n", "&"});

  started = false;
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
    openhd::log::get_default()->warn(
        "openhd needs 5G hotspot but hotspot card only supports 2G");
    openhd::log::get_default()->warn("Using 2.4G hotspot");
    should_use_5G = false;
  }
  return should_use_5G;
}

uint16_t WifiHotspot::get_frequency() {
  if (m_use_5G_channel) return 5180;
  return 2412;
}
