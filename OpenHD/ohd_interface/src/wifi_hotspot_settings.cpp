//
// Created by consti10 on 10.12.22.
//

#include "wifi_hotspot_settings.h"

WifiHotspotSettingsHolder::WifiHotspotSettingsHolder()
    :openhd::settings::PersistentSettings<WifiHotspotSettings>(openhd::INTERFACE_SETTINGS_DIRECTORY){
  init();
}

void wifi_hotspot_fixup_settings(
    WifiHotspotSettingsHolder& wifi_hotspot_settings_holder,const WiFiCard& wifi_card,
    const openhd::Space& wifibroadcast_frequency_space) {
  const bool wifibroadcast_uses_5G=wifibroadcast_frequency_space==openhd::Space::G5_8;
  if(wifibroadcast_uses_5G==wifi_hotspot_settings_holder.get_settings().use_5g_channel ){
    openhd::log::get_default()->warn("Switching hotspot frequency to not interfere with wb");
    if(wifibroadcast_uses_5G){
      wifi_hotspot_settings_holder.unsafe_get_settings().use_5g_channel= false;
    }else{
      wifi_hotspot_settings_holder.unsafe_get_settings().use_5g_channel= true;
    }
    wifi_hotspot_settings_holder.persist();
  }
}
