#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "json.hpp"

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"

#include "DProfile.h"

DProfile::DProfile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, int camera_count) :
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type),
    m_camera_count(camera_count) {}


std::string DProfile::generate_unit_id() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::ofstream of("/conf/openhd/unit.id");
    if (of) {
        of << uuid;
    }
    return boost::lexical_cast<std::string>(uuid);
}


void DProfile::discover() {
    std::cout << "Profile::discover()" << std::endl;
    std::ifstream unit_id_file("/conf/openhd/unit.id");
    if (!unit_id_file.is_open()) {
        m_unit_id = generate_unit_id();
        return;
    }
    std::string unit_id((std::istreambuf_iterator<char>(unit_id_file)),
                         std::istreambuf_iterator<char>());
    if (unit_id.empty() > 0) {
        m_unit_id = generate_unit_id();
        return;
    }
    m_unit_id = unit_id;
}


nlohmann::json DProfile::generate_manifest() {
    const bool is_air = m_camera_count > 0 ? true : false;
    OHDProfile ohdProfile{is_air,m_unit_id};
    return generate_profile_manifest(ohdProfile);
}
