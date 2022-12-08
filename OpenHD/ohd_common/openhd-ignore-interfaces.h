//
// Created by consti10 on 08.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_

#include "openhd-util.hpp"

// For developers
// By using the file specified here you can tell openhd to ignore certain interfaces
// This can be usefully if you want to create some kind of custom networking or similar
namespace openhd::ignore{

static constexpr auto filename="/boot/openhd/ignore_cards.txt";

static bool should_be_ignored(const std::string interface_name){
  if(OHDFilesystemUtil::exists(filename)){
    return false;
  }
  const auto content=OHDFilesystemUtil::read_file(filename);
  if(content.find(interface_name) != std::string::npos){
    return true;
  }
  return false;
}

}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_IGNORE_INTERFACES_H_
