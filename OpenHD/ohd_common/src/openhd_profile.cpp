//
// Created by consti10 on 10.02.23.
//

#include "openhd_profile.h"

#include <fstream>
#include <sstream>

#include "openhd_settings_directories.hpp"

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest.txt";

void write_profile_manifest(const OHDProfile& ohdProfile) {
  std::stringstream ss;
  ss<<"AIR:"<<(ohdProfile.is_air ? "YES" : "NO")<<"\n";
  ss<<"UNIT_ID:"<<ohdProfile.unit_id<<"\n";
  OHDFilesystemUtil::write_file(PROFILE_MANIFEST_FILENAME,ss.str());
}

std::shared_ptr<OHDProfile> DProfile::discover(bool is_air) {
  openhd::log::get_default()->debug("Profile::discover()");
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  const auto unit_id = openhd::getOrCreateUnitId();
  // We are air pi if there is at least one camera
  auto ret=std::make_shared<OHDProfile>(is_air,unit_id);
  return ret;
}
