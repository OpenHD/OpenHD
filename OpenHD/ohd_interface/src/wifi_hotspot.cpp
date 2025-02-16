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
 
 WifiHotspot::WifiHotspot(OHDProfile profile, WiFiCard wifiCard,
                          const openhd::WifiSpace& wifibroadcast_frequency_space)
     : m_profile(std::move(profile)), m_wifi_card(std::move(wifiCard)),
       m_wifibroadcast_frequency_space(wifibroadcast_frequency_space),
       m_console(openhd::log::create_or_get("wifi_hs")) {
   // Do not prepare hotspot here, just store the arguments
   m_console->warn("WifiHotspot object created but hotspot not initialized yet.");
 }
 
 WifiHotspot::~WifiHotspot() { }
 
 void WifiHotspot::start() {
   m_use_5G_channel = get_use_5g_channel(m_wifi_card, m_wifibroadcast_frequency_space);
   m_console->warn("Starting WIFI hotspot with the following parameters:");
   m_console->warn("Device Name: {}", m_wifi_card.device_name);
   m_console->warn("Supports 5GHz: {}", m_wifi_card.supports_5GHz() ? "Yes" : "No");
   m_console->warn("Using 5GHz Channel: {}", m_use_5G_channel ? "Yes" : "No");
   
   std::cout << blue << "Started WIFI hotspot on card "
             << m_wifi_card.device_name << " (5GHz: "
             << (m_use_5G_channel ? "Yes" : "No")
             << ")" << reset << std::endl;
 }
 
 void WifiHotspot::stop() {
   m_console->warn("Stopping wifi hotspot on card {}", m_wifi_card.device_name);
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
         "openhd needs 5G hotspot but hotspot card only supports 2G, you'll get really bad interference");
     openhd::log::get_default()->warn("Using 2.4G hotspot");
     should_use_5G = false;
   }
   return should_use_5G;
 }
 
 uint16_t WifiHotspot::get_frequency() {
   if (m_use_5G_channel) return 5180;
   return 2412;
 }
 