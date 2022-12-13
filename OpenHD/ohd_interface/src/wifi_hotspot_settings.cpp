//
// Created by consti10 on 10.12.22.
//

#include "wifi_hotspot_settings.h"

WifiHotspotSettingsHolder::WifiHotspotSettingsHolder()
    :openhd::settings::PersistentSettings<WifiHotspotSettings>(INTERFACE_SETTINGS_DIRECTORY){
  init();
}

void wifi_hotspot_fixup_settings(WifiHotspotSettingsHolder& wifi_hotspot_settings_holder,
                                 const WiFiCard& wifi_card) {
}
