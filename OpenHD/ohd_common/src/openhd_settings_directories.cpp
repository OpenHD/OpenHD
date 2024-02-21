//
// Created by consti10 on 21.02.24.
//

#include "openhd_settings_directories.h"

#include <cassert>
#include <utility>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

std::string openhd::getOrCreateUnitId() {
  generateSettingsDirectoryIfNonExists();
  auto unit_id_opt = OHDFilesystemUtil::opt_read_file(get_unit_id_file_path());
  if (unit_id_opt.has_value()) {
    std::string unit_id = unit_id_opt.value();
    // openhd::log::get_default()->debug("Read unit id:{}",unit_id);
    return unit_id;
  }
  // No unit id exists yet - create new one
  // generate new unit id
  // See https://www.boost.org/doc/libs/1_62_0/libs/uuid/uuid.html
  // const boost::uuids::uuid _uuid = boost::uuids::random_generator()();
  // unit_id = to_string(_uuid);
  std::string unit_id = create_unit_it_temporary();
  OHDFilesystemUtil::write_file(get_unit_id_file_path(), unit_id);
  openhd::log::get_default()->info("Created new unit id:{}", unit_id);
  return unit_id;
}

void openhd::clean_all_settings() {
  openhd::log::get_default()->debug("clean_all_settings()");
  OHDFilesystemUtil::safe_delete_directory(SETTINGS_BASE_PATH);
  generateSettingsDirectoryIfNonExists();
}

void openhd::check_currently_running_file_and_write() {
  if (OHDFilesystemUtil::exists(get_openhd_is_running_filename())) {
    openhd::log::get_default()->warn(
        "OpenHD is either still running in another process or did not "
        "terminate properly last time");
  }
  OHDFilesystemUtil::write_file(get_openhd_is_running_filename(), "dummy");
}

void openhd::remove_currently_running_file() {
  openhd::log::get_default()->debug(
      "OpenHD terminating,removing is running file");
  OHDFilesystemUtil::remove_if_existing(get_openhd_is_running_filename());
}

void openhd::generateSettingsDirectoryIfNonExists() {
  if (!OHDFilesystemUtil::exists(SETTINGS_BASE_PATH)) {
    OHDFilesystemUtil::create_directories(SETTINGS_BASE_PATH);
  }
  assert(OHDFilesystemUtil::exists(SETTINGS_BASE_PATH));
  OHDFilesystemUtil::create_directories(get_interface_settings_directory());
  OHDFilesystemUtil::create_directories(get_telemetry_settings_directory());
  OHDFilesystemUtil::create_directories(get_video_settings_directory());
}
