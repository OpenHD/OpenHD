//
// Created by consti10 on 04.01.23.
//

#include "ethernet_hotspot.h"

#include <openhd-util.hpp>
#include <utility>

static constexpr auto OHD_ETHERNET_HOTSPOT_CONNECTION_NAME ="ohd_eth_hotspot";

static void create_ethernet_hotspot_connection(const std::string& eth_device_name){
  // sudo nmcli con add type ethernet con-name "ohd_ethernet_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
  // sudo nmcli con add type ethernet ifname eth0 con-name ohd_eth_hotspot autoconnect no
  // sudo nmcli con modify ohd_eth_hotspot ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1

  // delete any already existing
  OHDUtil::run_command("nmcli",{"con","delete", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME});
  // and create the connection - note that the autoconnect is off by purpose
  OHDUtil::run_command("nmcli",{"con add type ethernet ifname",eth_device_name,"con-name", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,"autoconnect no"});
  OHDUtil::run_command("nmcli",{"con modify ", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,"ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1"});
}

EthernetHotspot::EthernetHotspot(std::string  device):m_device(std::move(device)) {
  m_console = openhd::log::create_or_get("wifi_hs");
  m_console->debug("begin create hotspot connection");
  create_ethernet_hotspot_connection(m_device);
  m_console->debug("end create hotspot connection");
  if(m_settings->get_settings().enable){
    start_async();
  }
}

void EthernetHotspot::start_async() {

}

void EthernetHotspot::stop_async() {

}

std::vector<openhd::Setting> EthernetHotspot::get_all_settings() {
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
  ret.push_back(openhd::Setting{"I_ETH_HOTSPOT_E",openhd::IntSetting{settings.enable,cb_enable}});
  return ret;
}
