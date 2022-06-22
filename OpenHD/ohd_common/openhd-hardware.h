//
// Created by consti10 on 22.06.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_HARDWARE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_HARDWARE_H_

#include "../ohd_common/openhd-camera.hpp"
#include "../ohd_common/openhd-platform.hpp"
#include "../ohd_common/openhd-profile.hpp"
#include "../ohd_common/openhd-wifi.hpp"

// Access to all the discovered OpenHD hardware
struct OHDHardware{
  std::shared_ptr<OHDPlatform> platform;
  std::shared_ptr<OHDProfile> profile;
  std::vector<std::shared_ptr<Camera>> cameras;
  std::vector<std::shared_ptr<WiFiCard>> wifiCards;
};


#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_HARDWARE_H_
