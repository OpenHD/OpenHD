//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_

#include <cstdint>

#include "json.hpp"
#include "openhd-settings-directories.hpp"
#include "openhd-settings-persistent.hpp"
#include "wifi_card.hpp"
#include "wifi_hotspot_settings.h"

struct EthernetHotspotSettings {
  bool enable=false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EthernetHotspotSettings,enable);

class EthernetHotspotSettingsHolder:public openhd::settings::PersistentSettings<WifiHotspotSettings>{
 public:
  EthernetHotspotSettingsHolder():
    openhd::settings::PersistentSettings<WifiHotspotSettings>(openhd::INTERFACE_SETTINGS_DIRECTORY){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return "ethernet_hotspot_settings.json";
  }
  [[nodiscard]] WifiHotspotSettings create_default()const override{
    return WifiHotspotSettings{};
  }
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_
