//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_

#include <cstdint>

#include "include_json.hpp"
#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.h"

struct EthernetHotspotSettings {
  bool enable=false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EthernetHotspotSettings,enable);

class EthernetHotspotSettingsHolder:public openhd::settings::PersistentSettings<EthernetHotspotSettings>{
 public:
  EthernetHotspotSettingsHolder():
    openhd::settings::PersistentSettings<EthernetHotspotSettings>(openhd::get_interface_settings_directory()){
    init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return "ethernet_hotspot_settings.json";
  }
  [[nodiscard]] EthernetHotspotSettings create_default()const override{
    return EthernetHotspotSettings{};
  }
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HOTSPOT_SETTINGS_H_
