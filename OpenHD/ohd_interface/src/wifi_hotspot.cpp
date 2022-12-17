//
// Created by consti10 on 17.05.22.
//

#include "wifi_hotspot.h"

#include <ISettingsComponent.hpp>
#include <utility>

#include "openhd-spdlog.hpp"

WifiHotspot::WifiHotspot(WiFiCard wifiCard):
m_wifi_card(std::move(wifiCard)) {
  m_settings=std::make_unique<WifiHotspotSettingsHolder>();
  wifi_hotspot_fixup_settings(*m_settings,m_wifi_card);
  if(m_settings->get_settings().enable){
    start();
  }
}

void WifiHotspot::start() {
  openhd::log::get_default()->debug("Starting WIFI hotspot on card {}",m_wifi_card.device_name);
  OHDUtil::run_command("nmcli",{"dev wifi hotspot ifname",m_wifi_card.device_name,"ssid openhd password \"openhdopenhd\""});
  started= true;
  openhd::log::get_default()->info("Wifi hotspot started");
}

void WifiHotspot::stop() {
  if(_start_async_thread){
    if(_start_async_thread->joinable()){
      _start_async_thread->join();
    }
    _start_async_thread=nullptr;
  }
  if(!started)return;
  // TODO: We turn wifi completely off in network manager here, but this should work / not interfere with the monitor mode card(s) since they are
  // not managed by network manager
  OHDUtil::run_command("nmcli",{"radio","wifi","off"});
}

void WifiHotspot::start_async() {
  if(_start_async_thread!= nullptr)return;
  _start_async_thread=std::make_unique<std::thread>(&WifiHotspot::start, this);
}

void WifiHotspot::stop_async() {
  if(_stop_async_thread!= nullptr)return;
  _stop_async_thread=std::make_unique<std::thread>(&WifiHotspot::stop, this);
}

std::vector<openhd::Setting> WifiHotspot::get_all_settings() {
  using namespace openhd;
  std::vector<openhd::Setting> ret{};
  const auto settings=m_settings->get_settings();
  auto cb_enable=[this](std::string,int value){
    if(!validate_yes_or_no(value))return false;
    m_settings->unsafe_get_settings().enable=value;
    m_settings->persist();
    if(m_settings->get_settings().enable){
      start();
    }else{
      stop();
    }
  };
  ret.push_back(openhd::Setting{"E_WIFI_HOTSPOT",openhd::IntSetting{settings.enable,cb_enable}});
  return ret;
}
