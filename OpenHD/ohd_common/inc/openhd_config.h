//
// Created by consti10 on 17.02.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_

namespace openhd{

struct Config{
  bool force_dummy_camera=false;
  bool force_custom_unmanaged_camera=false;
  bool force_ip_camera=false;
};

Config load_config();

void debug_config(const Config& config);

void persist_config(const Config& config);

}

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
