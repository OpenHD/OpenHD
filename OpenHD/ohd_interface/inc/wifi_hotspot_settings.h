//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_

#include <cstdint>

#include "include_json.hpp"
#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.hpp"
#include "wifi_card.hpp"

// NOTE: wb_link intentionally has its own settings
struct WifiHotspotSettings {
  bool enable=false;
  // weather to use any 2.4G or any 5G channel
  bool use_5g_channel=false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WifiHotspotSettings,enable,use_5g_channel);

class WifiHotspotSettingsHolder:public openhd::settings::PersistentSettings<WifiHotspotSettings>{
 public:
  WifiHotspotSettingsHolder();
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    std::stringstream ss;
    ss<<"wifi_hotspot_settings.json";
    return ss.str();
  }
  [[nodiscard]] WifiHotspotSettings create_default()const override{
    return WifiHotspotSettings{};
  }
};

void wifi_hotspot_fixup_settings(WifiHotspotSettingsHolder& wifi_hotspot_settings_holder,const WiFiCard& wifi_card,const openhd::Space& wifibroadcast_frequency_space);

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_
