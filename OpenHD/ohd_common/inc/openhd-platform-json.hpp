//
// Created by consti10 on 07.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_JSON_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_JSON_H_

#include "openhd-platform.hpp"
#include "include_json.hpp"

// Thw write out here is only for debugging
static void to_json(nlohmann::json& j,const OHDPlatform &ohdPlatform) {
  j = nlohmann::json{{"platform_type", platform_type_to_string(ohdPlatform.platform_type)},
                     {"board_type", board_type_to_string(ohdPlatform.board_type)}};
}

static constexpr auto PLATFORM_MANIFEST_FILENAME = "/tmp/platform_manifest";

static void write_platform_manifest(const OHDPlatform &ohdPlatform) {
  nlohmann::json manifest = ohdPlatform;
  std::ofstream _t(PLATFORM_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_JSON_H_
