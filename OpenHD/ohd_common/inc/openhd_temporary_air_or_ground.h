//
// Created by consti10 on 28.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_

#include "openhd_util_filesystem.h"

// Dirty, temporary
namespace openhd::tmp {

// Note: case sensitive
static constexpr auto FILENAME_AIR = "/boot/openhd/air.txt";
static constexpr auto FILENAME_GROUND = "/boot/openhd/ground.txt";

static bool file_air_exists() {
  return OHDFilesystemUtil::exists(FILENAME_AIR);
}
static bool file_ground_exists() {
  return OHDFilesystemUtil::exists(FILENAME_GROUND);
}

static bool file_air_or_ground_exists() {
  return file_air_exists() || file_ground_exists();
}

static void delete_any_file_air_or_ground() {
  OHDFilesystemUtil::remove_if_existing(FILENAME_AIR);
  OHDFilesystemUtil::remove_if_existing(FILENAME_GROUND);
}

static void write_file_air() {
  OHDFilesystemUtil::create_directories("/boot/openhd/");
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_AIR, " ");
}

static void write_file_ground() {
  OHDFilesystemUtil::create_directories("/boot/openhd/");
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_GROUND, " ");
}

static bool handle_telemetry_change(int value) {
  // 0==ground, 1==air, other: undefined (rejected)
  if (!(value == 0 || value == 1)) return false;
  if (value == 0) {
    // change to ground mode. Remove any existing file(s) if there are any
    openhd::tmp::delete_any_file_air_or_ground();
    openhd::tmp::write_file_ground();
  } else {
    // change to air mode. Remove any existing file(s) if there are any
    openhd::tmp::delete_any_file_air_or_ground();
    openhd::tmp::write_file_air();
  }
  return true;
}

}  // namespace openhd::tmp
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
