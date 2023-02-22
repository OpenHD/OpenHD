//
// Created by consti10 on 17.02.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_

#include <string>
#include <vector>

namespace openhd{

// NOTE: Read the .config - file itself for documentation what these variables do
struct Config{
  // WIFI
  bool WIFI_ENABLE_AUTODETECT= true;
  std::vector<std::string> WIFI_WB_LINK_CARDS{};
  std::string WIFI_WIFI_HOTSPOT_CARD;
  // CAMERAS
  bool CAMERA_ENABLE_AUTODETECT = true;
  int CAMERA_N_CAMERAS;
  std::string CAMERA_CAMERA0_TYPE;
  std::string CAMERA_CAMERA1_TYPE;
};

Config load_config();

void debug_config(const Config& config);

}

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
