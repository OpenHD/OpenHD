//
// Created by consti10 on 21.05.22.
//

#include <csignal>
#include <thread>

#include "openhd-util.hpp"
#include "wifi_hotspot.h"

int main(int argc, char *argv[]) {

  OHDUtil::terminate_if_not_root();

  WiFiCard wifiCard;
  // need to manually paste the stuff in here
  //wifiCard.interface_name="wlx244bfeb71c05";
  //wifiCard.mac="24:4b:fe:b7:1c:05";
  wifiCard.device_name ="wlan0";
  wifiCard.mac = "e4:5f:01:b0:55:92";
  wifiCard.supports_hotspot= true;

  WifiHotspot wifiHotspot{wifiCard,openhd::Space::G2_4};
  OHDUtil::keep_alive_until_sigterm();
  std::cout<<"test end\n";
  return 0;

}
