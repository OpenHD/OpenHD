//
// Created by consti10 on 07.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_

#include "openhd-spdlog.hpp"
#include "openhd-util-filesystem.hpp"

// Quick helper methods for ethernet (used for automatic data forwarding detection)
namespace openhd::ethernet{

// Check if the given ethernet device is in an "up" state by reading linux file(s)
static bool check_eth_adapter_up(const std::string& eth_device_name="eth0"){
  const auto filename_operstate=fmt::format("/sys/class/net/{}/operstate",eth_device_name);
  if(!OHDFilesystemUtil::exists(filename_operstate.c_str()))return false;
  const auto content_opt=OHDFilesystemUtil::opt_read_file(filename_operstate);
  if(!content_opt.has_value())return false;
  const auto& content=content_opt.value();
  if(OHDUtil::contains(content,"up")){
    return true;
  }
  return false;
}

}
#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_
