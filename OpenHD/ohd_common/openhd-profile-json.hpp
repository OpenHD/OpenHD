//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_PROFILE_JSON_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_PROFILE_JSON_H_

#include "openhd-profile.hpp"
#include "include_json.hpp"

// Thw write out here is only for debugging
static void to_json(nlohmann::json& j, const OHDProfile& p) {
  j = nlohmann::json{ {"is_air", p.is_air},{"started_from_service", p.started_from_service}, {"unit_id", p.unit_id}};
}

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest";

static void write_profile_manifest(const OHDProfile &ohdProfile) {
  nlohmann::json manifest = ohdProfile;
  std::ofstream _t(PROFILE_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_PROFILE_JSON_H_
