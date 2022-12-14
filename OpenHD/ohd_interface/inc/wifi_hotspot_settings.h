//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_

#include <cstdint>

#include "json.hpp"
#include "openhd-settings-directories.hpp"
#include "openhd-settings-persistent.hpp"
#include "wifi_card.hpp"

// NOTE: wb_link intentionally has its own settings
struct WifiHotspotSettings {
  bool enable=false;
  uint32_t frequency=2412;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WifiHotspotSettings,enable,frequency);

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

void wifi_hotspot_fixup_settings(WifiHotspotSettingsHolder& wifi_hotspot_settings_holder,const WiFiCard& wifi_card);

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_HOTSPOT_SETTINGS_H_
