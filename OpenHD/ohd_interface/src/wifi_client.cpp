//
// Created by consti10 on 30.01.24.
//

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
  // Disble the wifi hotspot if needed
  if (WifiHotspot::util_delete_nm_file()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  const auto command = create_command_wifi_client(
      config.WIFI_LOCAL_NETWORK_SSID, config.WIFI_LOCAL_NETWORK_PASSWORD);
  OHDUtil::run_command(command, {}, true);
  return true;
}
