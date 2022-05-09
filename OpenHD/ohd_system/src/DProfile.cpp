#include <iostream>
#include <sstream>

#include "json.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "openhd-settings.hpp"

#include "DProfile.h"

DProfile::DProfile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, int camera_count) :
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type),
    m_camera_count(camera_count) {}



void DProfile::discover() {
    std::cout << "Profile::discover()" << std::endl;
    // We read the unit id from the persistent storage, later write it to the tmp storage json
    m_unit_id = getOrCreateUnitId();
}


nlohmann::json DProfile::generate_manifest() {
    const bool is_air = m_camera_count > 0 ? true : false;
    OHDProfile ohdProfile{is_air,m_unit_id};
    return generate_profile_manifest(ohdProfile);
}
