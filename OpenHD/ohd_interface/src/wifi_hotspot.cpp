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
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>
#include "openhd_action_handler.h"
#include <utility>

#include "openhd_naming.h"
#include "openhd_spdlog.h"
#include "openhd_util_async.h"

static constexpr auto OHD_WIFI_HOTSPOT_CONNECTION_NAME = "ohd_wifi_hotspot";
const std::string blue = "\033[34m";
const std::string reset = "\033[0m";

static std::string get_ohd_wifi_hotspot_connection_nm_filename() {
  return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",
                     OHD_WIFI_HOTSPOT_CONNECTION_NAME);
}

static std::string resolve_hotspot_ssid(bool is_air,
                                        const std::string& override_ssid) {
  if (override_ssid.empty()) {
    return openhd::naming::build_unit_name(is_air);
  }
  if (override_ssid.size() > 32) {
    openhd::log::get_default()->warn(
        "Hotspot SSID override too long ({}), using default",
        override_ssid.size());
    return openhd::naming::build_unit_name(is_air);
  }
  return override_ssid;
}

static std::string resolve_hotspot_password(
    const std::string& override_password) {
  if (override_password.empty()) {
    return "openhdopenhd";
  }
  if (override_password.size() < 8 || override_password.size() > 63) {
    openhd::log::get_default()->warn(
        "Hotspot password override invalid length ({}), using default",
        override_password.size());
    return "openhdopenhd";
  }
  return override_password;
}

// NOTE: This creates the proper NM connection, but does not start it yet.
static bool create_hotspot_connection_file(
    const WiFiCard& card, const bool is_air, const bool use_5g_channel,
    const std::string& ssid_override, const std::string& password_override) {
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
  const auto ssid = resolve_hotspot_ssid(is_air, ssid_override);
  const auto password = resolve_hotspot_password(password_override);
  OHDUtil::run_command(
      "nmcli", {"con add type wifi ifname", card.device_name, "con-name",
                OHD_WIFI_HOTSPOT_CONNECTION_NAME, "autoconnect no",
                fmt::format("ssid {}", ssid)});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        " 802-11-wireless.mode ap", "802-11-wireless.band",
                        use_5g_channel ? "a" : "bg",
                        "802-11-wireless.channel",
                        use_5g_channel ? "36" : "1", "ipv4.method shared"});
  // Hotspot clients are trusted local users. Marking this connection trusted
  // prevents firewalld from rejecting forwarding to another local interface,
  // notably the Ethernet subnet used by IP cameras and their web interfaces.
  // NetworkManager's shared IPv4 mode supplies forwarding and masquerading so
  // the camera does not need a route back to 192.168.3.0/24.
  OHDUtil::run_command(
      "nmcli", {"con modify", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                "connection.zone trusted", "ipv4.never-default yes",
                "ipv6.method disabled"});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        " wifi-sec.key-mgmt wpa-psk"});
  OHDUtil::run_command("nmcli",
                       {"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME,
                        fmt::format(" wifi-sec.psk \"{}\"", password)});
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
                         const openhd::WifiSpace& wifibroadcast_frequency_space,
                         std::string hotspot_ssid_override,
                         std::string hotspot_password_override)
    : m_profile(std::move(profile)),
      m_wifi_card(std::move(wifiCard)),
      m_hotspot_ssid_override(std::move(hotspot_ssid_override)),
      m_hotspot_password_override(std::move(hotspot_password_override)) {
  m_use_5G_channel = WifiHotspot::get_use_5g_channel(
      m_wifi_card, wifibroadcast_frequency_space);
  m_console = openhd::log::create_or_get("wifi_hs");
  // create the connection (no matter if hotspot is enabled) such that we can
  // just enable / disable it by running connection up / down.
  m_console->debug("begin create hotspot connection");
  m_use_buildroot = access("/usr/sbin/openhd-pico-hotspot", X_OK) == 0;
  if (m_use_buildroot) m_use_5G_channel = false;
  if (!m_use_buildroot) create_hotspot_connection_file(m_wifi_card, m_profile.is_air,
                                 m_use_5G_channel, m_hotspot_ssid_override,
                                 m_hotspot_password_override);
  m_console->debug("end create hotspot connection");
}

WifiHotspot::~WifiHotspot() {
  if (m_operation.valid()) m_operation.wait();
  stop();
  if (!m_use_buildroot) util_delete_nm_file();
}

void WifiHotspot::start() {
  int result = -1;
  if (m_use_buildroot) {
    const auto ssid = resolve_hotspot_ssid(m_profile.is_air, m_hotspot_ssid_override);
    const auto password = resolve_hotspot_password(m_hotspot_password_override);
    if (password.find_first_of("\r\n") != std::string::npos) return;
    std::filesystem::create_directories("/run/openhd");
    const char* config = "/run/openhd/pico-hostapd.conf";
    // Never put credentials into shell arguments or command logs.
    std::ofstream file(config);
    chmod(config, 0600);
    file << "interface=" << m_wifi_card.device_name
         << "\ndriver=nl80211\nctrl_interface=/run/openhd/hostapd\nssid2=";
    static constexpr char hex[] = "0123456789abcdef";
    for (unsigned char c : ssid) file << hex[c >> 4] << hex[c & 15];
    file << "\nhw_mode=g\nchannel=1\nieee80211n=1\nwmm_enabled=1"
         << "\nwpa=2\nwpa_key_mgmt=WPA-PSK\nrsn_pairwise=CCMP\nwpa_passphrase="
         << password << "\n";
    file.close();
    if (file) result = OHDUtil::run_command("/usr/sbin/openhd-pico-hotspot", {"start"}, false);
  } else {
    result = OHDUtil::run_command("nmcli", {"con", "up", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  }
  started = result == 0;
  auto& actions = openhd::LinkActionHandler::instance();
  actions.m_wifi_hotspot_state = started ? 2 : 1;
  actions.m_wifi_hotspot_frequency = started ? get_frequency() : 0;
  if (started) m_console->info("Wifi hotspot started");
  else m_console->error("Wifi hotspot did not start");
}

void WifiHotspot::stop() {
  if (!started) return;
  const int result = m_use_buildroot
      ? OHDUtil::run_command("/usr/sbin/openhd-pico-hotspot", {"stop"}, false)
      : OHDUtil::run_command("nmcli", {"con", "down", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  if (result == 0) started = false;
  auto& actions = openhd::LinkActionHandler::instance();
  actions.m_wifi_hotspot_state = started ? 2 : 1;
  actions.m_wifi_hotspot_frequency = started ? get_frequency() : 0;
}

void WifiHotspot::start_async() {
  m_operation = std::async(std::launch::async, [this]() { start(); });
}
void WifiHotspot::stop_async() {
  m_operation = std::async(std::launch::async, [this]() { stop(); });
}

void WifiHotspot::set_enabled_async(bool enable) {
  if (m_is_enabled == enable) return;
  if (m_operation.valid()) m_operation.wait();
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
