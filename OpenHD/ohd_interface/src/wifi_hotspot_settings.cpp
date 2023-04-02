//
// Created by consti10 on 10.12.22.
//

#include "wifi_hotspot_settings.h"

WifiHotspotSettingsHolder::WifiHotspotSettingsHolder()
    :openhd::PersistentJsonSettings<WifiHotspotSettings>(openhd::get_interface_settings_directory()){
  init();
}

void wifi_hotspot_fixup_settings(
    WifiHotspotSettingsHolder& wifi_hotspot_settings_holder,const WiFiCard& wifi_card,
    const openhd::WifiSpace& wifibroadcast_frequency_space) {
  // openhd definition: hotspot always run on the opposite spectrum of wifibroadcast
  const bool wifibroadcast_uses_5G=wifibroadcast_frequency_space==openhd::WifiSpace::G5_8;
  if(wifibroadcast_uses_5G==wifi_hotspot_settings_holder.get_settings().use_5g_channel ){
    openhd::log::get_default()->warn("Switching hotspot frequency to not interfere with wb");
    if(wifibroadcast_uses_5G){
      wifi_hotspot_settings_holder.unsafe_get_settings().use_5g_channel= false;
    }else{
      wifi_hotspot_settings_holder.unsafe_get_settings().use_5g_channel= true;
    }
    wifi_hotspot_settings_holder.persist();
  }
  if(wifi_hotspot_settings_holder.get_settings().use_5g_channel && !wifi_card.supports_5GHz()){
    openhd::log::get_default()->warn("openhd needs 5G hotspot but hotspot card only supports 2G,you'l get really bad interference");
    wifi_hotspot_settings_holder.unsafe_get_settings().use_5g_channel= false;
    wifi_hotspot_settings_holder.persist();
  }
}
