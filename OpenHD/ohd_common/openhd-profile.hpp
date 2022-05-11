#ifndef OPENHD_PROFILE_H
#define OPENHD_PROFILE_H


#include <string>
#include <vector>
#include <sstream>

#include "json.hpp"
#include "openhd-util.hpp"
#include "openhd-log.hpp"

/**
 * The profile is created on startup and then NEVER CHANGES !.
 * Note that while the unit id never changes between successive re-boots of OpenHD,
 * the is_air variable might change (aka a ground pi might become an air pi when the user switches things around, or opposite).
 */
struct OHDProfile{
    // Weather we run on an air or ground "pi" (air or ground system).
    // R.n this is determined by weather there is at least one camera connected to the system
    // TODO or if there is a file air.txt, for debugging.
    bool is_air= false;
    // The unique id of this system, it is created once then never changed again.
    std::string unit_id="0";
};

static nlohmann::json generate_profile_manifest(const OHDProfile& ohdProfile) {
    nlohmann::json j;
    j["unit-id"] = ohdProfile.unit_id;
    j["is-air"] = ohdProfile.is_air;
    std::stringstream message;
    message<<"OHDProfile:is_air:"<<(ohdProfile.is_air ? "Y" : "N")<<" unit_id:"<<ohdProfile.unit_id<<"\n";
    ohd_log(STATUS_LEVEL_INFO, message.str());
    return j;
}

static constexpr auto PROFILE_MANIFEST_FILENAME="/tmp/profile_manifest";

// The to/from json are mostly a microservices artifact.
// NOTE: It is only safe to call this method after the discovery step.
static OHDProfile profile_from_manifest() {
    try {
        std::ifstream f(PROFILE_MANIFEST_FILENAME);
        nlohmann::json j;
        f >> j;
        OHDProfile profile;
        profile.is_air = j["is-air"];
        profile.unit_id = j["unit-id"];
        return profile;
    }catch (std::exception &ex) {
        std::cerr << "Profile manifest processing failed: " << ex.what() << std::endl;
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        exit(1);
    }
}

#endif

