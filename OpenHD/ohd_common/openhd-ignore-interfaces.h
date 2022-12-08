//
// Created by consti10 on 08.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_

#include "openhd-util.hpp"

// For developers
// By using the file(s) specified here you can tell openhd to ignore certain interfaces
// This can be usefully if you want to create some kind of custom networking or similar
namespace openhd::ignore{

static constexpr auto filename_ignore_interfaces="/boot/openhd/ignore_interfaces.txt";
static constexpr auto filename_ignore_macs="/boot/openhd/ignore_macs.txt";

static bool should_be_ignored(const char* file_name,const std::string& evaluated){
  if(!OHDFilesystemUtil::exists(file_name)){
    return false;
  }
  const auto content=OHDFilesystemUtil::read_file(file_name);
  if(content.find(evaluated) != std::string::npos){
    return true;
  }
  return false;
}

static bool should_be_ignored_interface(const std::string& interface_name){
  return should_be_ignored(filename_ignore_interfaces,interface_name);
}

static bool should_be_ignored_mac(const std::string& mac){
  return should_be_ignored(filename_ignore_macs,mac);
}

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_
