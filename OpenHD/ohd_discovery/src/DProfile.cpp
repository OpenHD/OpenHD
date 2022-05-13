#include <iostream>
#include <sstream>

#include "json.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "openhd-settings.hpp"

#include "DProfile.h"

DProfile::DProfile(bool is_air) :
    m_is_air(is_air){}


void DProfile::discover() {
    std::cout << "Profile::discover()" << std::endl;
    // We read the unit id from the persistent storage, later write it to the tmp storage json
    m_unit_id = getOrCreateUnitId();
}

void DProfile::write_manifest() {
    OHDProfile ohdProfile{m_is_air,m_unit_id};
    write_profile_manifest(ohdProfile);
}
