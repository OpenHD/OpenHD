//
// Created by consti10 on 13.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_WIFI_CARDS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_WIFI_CARDS_H_

#include "wifi_card.hpp"
#include <optional>
#include "wifi_channel.h"
#include "manually_defined_cards.h"

namespace openhd{

struct XWifiCard{
  std::string interface_name;
  std::string mac_address;
  WifiUseFor usage;
  void discover_capabilities(){

  }
  std::vector<openhd::WifiChannel> supported_channels;
};

}








#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_WIFI_CARDS_H_
