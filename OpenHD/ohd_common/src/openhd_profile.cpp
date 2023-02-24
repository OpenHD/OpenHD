//
// Created by consti10 on 10.02.23.
//

#include "openhd_profile.h"

#include <fstream>

#include "include_json.hpp"
#include "openhd_settings_directories.hpp"

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest";

// Thw write out here is only for debugging
static void to_json(nlohmann::json& j, const OHDProfile& p) {
  j = nlohmann::json{ {"is_air", p.is_air}, {"unit_id", p.unit_id}};
}

void write_profile_manifest(const OHDProfile& ohdProfile) {
  nlohmann::json manifest = ohdProfile;
  std::ofstream _t(PROFILE_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

std::shared_ptr<OHDProfile> DProfile::discover(bool is_air) {
  openhd::log::get_default()->debug("Profile::discover()");
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  const auto unit_id = openhd::getOrCreateUnitId();
  // We are air pi if there is at least one camera
  auto ret=std::make_shared<OHDProfile>(is_air,unit_id);
  return ret;
}
