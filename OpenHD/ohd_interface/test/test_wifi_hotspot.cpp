//
// Created by consti10 on 21.05.22.
//

#include "WifiHotspot.h"
#include <thread>
#include <csignal>
#include "openhd-util.hpp"

int main(int argc, char *argv[]) {

  OHDUtil::terminate_if_not_root();

  WiFiCard wifiCard;
  // need to manually paste the stuff in here
  //wifiCard.interface_name="wlx244bfeb71c05";
  //wifiCard.mac="24:4b:fe:b7:1c:05";
  wifiCard.interface_name="wlan0";
  wifiCard.mac = "e4:5f:01:b0:55:92";
  wifiCard.supports_hotspot= true;

  WifiHotspot wifiHotspot{wifiCard};
  wifiHotspot.start();
  OHDUtil::keep_alive_until_sigterm();
  std::cout<<"test end\n";
  wifiHotspot.stop();
  return 0;

}
