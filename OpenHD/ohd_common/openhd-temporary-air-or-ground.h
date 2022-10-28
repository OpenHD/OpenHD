//
// Created by consti10 on 28.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_

#include "openhd-util-filesystem.hpp"

// Dirty, temporary
namespace openhd::tmp{

// Note: case sensitive
static constexpr auto FILENAME_AIR="/boot/OpenHD/air.txt";
static constexpr auto FILENAME_GROUND="/boot/OpenHD/ground.txt";

static bool file_air_exists(){
  return OHDFilesystemUtil::exists(FILENAME_AIR);
}
static bool file_ground_exists(){
  return OHDFilesystemUtil::exists(FILENAME_GROUND);
}

static bool file_air_or_ground_exists(){
  return file_air_exists() || file_ground_exists();
}

static void delete_any_file_air_or_ground(){
  OHDFilesystemUtil::remove_if_existing(FILENAME_AIR_1);
  OHDFilesystemUtil::remove_if_existing(FILENAME_GROUND_1);
}

static void write_file_air(){
  OHDFilesystemUtil::create_directories("/boot/OpenHD/");
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_AIR," ");
}

static void write_file_ground(){
  OHDFilesystemUtil::create_directories("/boot/OpenHD/");
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_GROUND," ");
}


}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
