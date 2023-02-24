//
// Created by consti10 on 17.05.22.
//

#include "wifi_hotspot.h"

#include <utility>

#include "openhd_spdlog.h"

static constexpr auto OHD_WIFI_HOTSPOT_CONNECTION_NAME ="ohd_wfi_hotspot";

// NOTE: This creates the proper NM connection, but does not start it yet.
static bool create_hotspot_connection(const WiFiCard& card,const WifiHotspotSettings& settings){
  // delete any previous connection that might exist. This might fail if no connection of that name exists -
  // aka an error here can be ignored
  OHDUtil::run_command("nmcli",{"con","delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
  // and create the hotspot one
  OHDUtil::run_command("nmcli",{"con add type wifi ifname",card.device_name,"con-name", OHD_WIFI_HOTSPOT_CONNECTION_NAME,"autoconnect no ssid openhd"});
  OHDUtil::run_command("nmcli",{"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME," 802-11-wireless.mode ap",
                                 "802-11-wireless.band",settings.use_5g_channel ? "a" : "bg",
                                 "ipv4.method shared"});
  OHDUtil::run_command("nmcli",{"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME," wifi-sec.key-mgmt wpa-psk"});
  OHDUtil::run_command("nmcli",{"con modify ", OHD_WIFI_HOTSPOT_CONNECTION_NAME," wifi-sec.psk \"openhdopenhd\""});
  OHDUtil::run_command("nmcli",{"con modify",OHD_WIFI_HOTSPOT_CONNECTION_NAME,"ipv4.addresses 192.168.3.1/24"});
  return true;
}


WifiHotspot::WifiHotspot(WiFiCard wifiCard,const openhd::WifiSpace& wifibroadcast_frequency_space):
m_wifi_card(std::move(wifiCard)) {
  m_console = openhd::log::create_or_get("wifi_hs");
  m_settings=std::make_unique<WifiHotspotSettingsHolder>();
  wifi_hotspot_fixup_settings(*m_settings,m_wifi_card,wifibroadcast_frequency_space);
  // create the connection (no matter if hotspot is enabled) such that we can just enable / disable it whenn the hotspot changes up/down
  m_console->debug("begin create hotspot connection");
  create_hotspot_connection(m_wifi_card,m_settings->get_settings());
  m_console->debug("end create hotspot connection");
  if(m_settings->get_settings().enable){
    start_async();
  }
}

WifiHotspot::~WifiHotspot() {
  // cleanup - proper stop of openhd, do not leave any traces behind.
  OHDUtil::run_command("nmcli",{"con", "delete", OHD_WIFI_HOTSPOT_CONNECTION_NAME});
}


void WifiHotspot::start() {
  m_console->debug("Starting WIFI hotspot on card {}",m_wifi_card.device_name);
  const auto args=std::vector<std::string>{"con","up", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli",args);
  started= true;
  m_console->info("Wifi hotspot started");
}

void WifiHotspot::stop() {
  m_console->debug("Stopping wifi hotspot on card {}",m_wifi_card.device_name);
  if(!started)return;
  const auto args=std::vector<std::string>{"con","down", OHD_WIFI_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli",args);
  m_console->info("Wifi hotspot stopped");
}

void WifiHotspot::start_async() {
  m_last_async_operation=std::async(std::launch::async, &WifiHotspot::start,this);
}

void WifiHotspot::stop_async() {
  m_last_async_operation=std::async(std::launch::async, &WifiHotspot::stop,this);
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
