//
// Created by consti10 on 10.02.23.
//

#include "openhd_profile.h"

#include <sstream>

#include "openhd_settings_directories.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest.txt";

void write_profile_manifest(const OHDProfile& ohdProfile) {
  std::stringstream ss;
  ss << "AIR:" << (ohdProfile.is_air ? "YES" : "NO") << "\n";
  ss << "UNIT_ID:" << ohdProfile.unit_id << "\n";
  OHDFilesystemUtil::write_file(PROFILE_MANIFEST_FILENAME, ss.str());
}

OHDProfile DProfile::discover(bool is_air) {
  openhd::log::get_default()->debug("Profile:[{}]", is_air ? "AIR" : "GND");
  // We read the unit id from the persistent storage, later write it to the tmp
  // storage json
  const auto unit_id = openhd::getOrCreateUnitId();
  return OHDProfile(is_air, unit_id);
}
