//
// Created by consti10 on 28.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_

#include "openhd-util-filesystem.hpp"

// Dirty, temporary
namespace openhd::tmp{

static constexpr auto FILENAME_AIR_1="/boot/OpenHD/air.txt";
static constexpr auto FILENAME_AIR_2="/boot/OpenHD/Air.txt";
static constexpr auto FILENAME_GROUND_1="/boot/OpenHD/ground.txt";
static constexpr auto FILENAME_GROUND_2="/boot/OpenHD/Ground.txt";

static bool any_file_air_exists(){
  return OHDFilesystemUtil::exists(FILENAME_AIR_1) ||
         OHDFilesystemUtil::exists(FILENAME_AIR_2);
}
static bool any_file_ground_exists(){
  return OHDFilesystemUtil::exists(FILENAME_GROUND_1) ||
         OHDFilesystemUtil::exists(FILENAME_GROUND_2);
}

static bool any_file_air_or_ground_exists(){
  return any_file_air_exists() || any_file_ground_exists();
}

static void delete_any_file_air_or_ground(){
  OHDFilesystemUtil::remove_if_existing(FILENAME_AIR_1);
  OHDFilesystemUtil::remove_if_existing(FILENAME_AIR_2);
  OHDFilesystemUtil::remove_if_existing(FILENAME_GROUND_1);
  OHDFilesystemUtil::remove_if_existing(FILENAME_GROUND_2);
}


}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
