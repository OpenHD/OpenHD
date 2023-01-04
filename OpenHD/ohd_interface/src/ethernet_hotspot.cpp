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

}
