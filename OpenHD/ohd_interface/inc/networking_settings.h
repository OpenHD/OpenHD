//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_

#include <cstdint>

#include "include_json.hpp"
#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

// Networking related settings, separate from wb_link
struct NetworkingSettings {
  // WIFI Hotspot, can be enabled / disabled at run time if an extra Wi-Fi hotspot card exists on the system
  // TODO: Auto warning if wifi hotspot is enabled & drone is armed ?
  bool wifi_hotspot_enable=false;
  // Ethernet hotspot (changes networking,requires reboot)
  bool ethernet_hotspot_enable=false;
  // passive listening for forwarding without hotspot functionality, can be enabled / disabled at run time.
  bool ethernet_nonhotspot_enable_auto_forwarding=false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkingSettings,wifi_hotspot_enable,ethernet_hotspot_enable,ethernet_nonhotspot_enable_auto_forwarding);

class NetworkingSettingsHolder:public openhd::PersistentJsonSettings<NetworkingSettings>{
 public:
  NetworkingSettingsHolder():
    openhd::PersistentJsonSettings<NetworkingSettings>(openhd::get_interface_settings_directory()){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return "networking_settings.json";
  }
  [[nodiscard]] NetworkingSettings create_default()const override{
    return NetworkingSettings{};
  }
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_NETWORKING_SETTINGS_H_
