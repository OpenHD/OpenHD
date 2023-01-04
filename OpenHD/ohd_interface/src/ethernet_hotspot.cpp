//
// Created by consti10 on 04.01.23.
//

#include "ethernet_hotspot.h"

#include <utility>

static void create_ethernet_hotspot_connection(){
  // sudo nmcli con add type ethernet con-name "ohd_ethernet_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
}

EthernetHotspot::EthernetHotspot(std::string  device):m_device(std::move(device)) {

}
