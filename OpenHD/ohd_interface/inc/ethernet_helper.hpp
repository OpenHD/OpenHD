//
// Created by consti10 on 07.01.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_

#include <regex>

#include "openhd_spdlog.hpp"
#include "openhd_util_filesystem.h"

// Quick helper methods for ethernet (used for automatic data forwarding detection)
namespace openhd::ethernet{

// Check if the given ethernet device is in an "up" state by reading linux file(s)
static bool check_eth_adapter_up(const std::string& eth_device_name="eth0"){
  const auto filename_operstate=fmt::format("/sys/class/net/{}/operstate",eth_device_name);
  if(!OHDFilesystemUtil::exists(filename_operstate))return false;
  const auto content_opt=OHDFilesystemUtil::opt_read_file(filename_operstate);
  if(!content_opt.has_value())return false;
  const auto& content=content_opt.value();
  if(OHDUtil::contains(content,"up")){
    return true;
  }
  return false;
}

// find / get the ip address in the given string with the following layout ...(192.168.2.158)... where
// "192.168.2.158" can be any ip address
static std::string get_ip_address_in_between_brackets(const std::string& s,const bool debug=false){
  const std::regex base_regex("\\((.*)\\)");
  std::smatch base_match;
  std::string matched;
  if (std::regex_search(s, base_match, base_regex)) {
    // The first sub_match is the whole string; the next
    // sub_match is the first parenthesized expression.
    if (base_match.size() == 2) {
      matched = base_match[1].str();
    }
  }
  if(debug){
    openhd::log::get_default()->debug("Given:[{}] Result:[{}]",s,matched);
  }
  return matched;
}

}
#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_ETHERNET_HELPER_HPP_
