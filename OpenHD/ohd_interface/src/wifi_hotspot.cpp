//
// Created by consti10 on 17.05.22.
//

#include "wifi_hotspot.h"

#include <future>
#include <utility>

#include "openhd-spdlog.hpp"

static constexpr auto OHD_HOTSPOT_CONNECTION_NAME="openhd_hotspot";

WifiHotspot::WifiHotspot(WiFiCard wifiCard):
m_wifi_card(std::move(wifiCard)) {
  m_settings=std::make_unique<WifiHotspotSettingsHolder>();
  wifi_hotspot_fixup_settings(*m_settings,m_wifi_card);
  if(m_settings->get_settings().enable){
    start_async();
  }
}

void WifiHotspot::start() {
  openhd::log::get_default()->debug("Starting WIFI hotspot on card {}",m_wifi_card.device_name);
  const auto args=std::vector<std::string>{
      "dev wifi hotspot",
       "con-name",OHD_HOTSPOT_CONNECTION_NAME,
       "ifname",m_wifi_card.device_name,
       "ssid","openhd",
       "password", "\"openhdopenhd\""
  };
  OHDUtil::run_command("nmcli",args);
  started= true;
  openhd::log::get_default()->info("Wifi hotspot started");
}

void WifiHotspot::stop() {
  openhd::log::get_default()->debug("Stopping wifi hotspot on card {}",m_wifi_card.device_name);
  if(!started)return;
  // TODO: We turn wifi completely off in network manager here, but this should work / not interfere with the monitor mode card(s) since they are
  // not managed by network manager
  const auto args=std::vector<std::string>{
      "con","down",OHD_HOTSPOT_CONNECTION_NAME
  };
  OHDUtil::run_command("nmcli",args);
  openhd::log::get_default()->info("Wifi hotspot stopped");
}

void WifiHotspot::start_async() {
  auto result=std::async(std::launch::async, &WifiHotspot::start,this);
}

void WifiHotspot::stop_async() {
  auto result=std::async(std::launch::async, &WifiHotspot::stop,this);
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
      start_async();
    }else{
      stop_async();
    }
    return true;
  };
  ret.push_back(openhd::Setting{"I_WIFI_HOTSPOT_E",openhd::IntSetting{settings.enable,cb_enable}});
  return ret;
}
