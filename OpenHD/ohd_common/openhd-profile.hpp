#ifndef OPENHD_PROFILE_H
#define OPENHD_PROFILE_H


#include <string>
#include <vector>

#include "openhd-util.hpp"


typedef enum UnitType {
    UnitTypeAir,
    UnitTypeGround
} UnitType;


struct OpenHDProfile {
    UnitType unit_type;
    std::string unit_id;
} OpenHDProfile;


inline OpenHDProfile get_profile() {
    std::ifstream f("/tmp/profile_manifest");
    nlohmann::json j;
    f >> j;
    OpenHDProfile profile;
    bool is_air = j["is-air"];
    profile.unit_type = is_air ? UnitTypeAir : UnitTypeGround;
    profile.unit_id = j["unit-id"];
    return profile;
}


#endif

