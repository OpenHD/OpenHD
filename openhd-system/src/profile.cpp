#include <cstdio>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"

#include "platform.h"
#include "profile.h"

extern "C" {
    #include <bcm2835.h>
}


Profile::Profile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, int camera_count) : 
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type),
    m_camera_count(camera_count) {}


std::string Profile::generate_unit_id() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    std::ofstream of("/conf/openhd/unit.id");
    if (of) {
        of << uuid;
    }

    return boost::lexical_cast<std::string>(uuid);
}


/*
 * This is a unique ID for the ground and air unit. 
 * 
 * When the air side first connects, it will announce itself with a service channel message that
 * contains this ID. The ground side can then use the ID to find the correct settings files for
 * that air unit, and if needed it will send them over. This takes the place of SmartSync and 
 * is automatic.
 * 
 */
void Profile::discover() {
    std::cout << "Profile::discover()" << std::endl;

    std::ifstream unit_id_file("/conf/openhd/unit.id");

    if (!unit_id_file.is_open()) {
        m_unit_id = generate_unit_id();
        return;
    }

    std::string unit_id((std::istreambuf_iterator<char>(unit_id_file)),
                         std::istreambuf_iterator<char>());

    if (!unit_id.size() > 0) {
        m_unit_id = generate_unit_id();
        return;
    }

    m_unit_id = unit_id;
}


nlohmann::json Profile::generate_manifest() {
    nlohmann::json j;
    
    j["unit-id"] = m_unit_id;

    bool is_air = m_camera_count > 0 ? true : false;
    j["is-air"] = is_air;

    // global sysid for microservices
    int sys_id = is_air ? 253 : 254;

    j["microservice-sys-id"] = sys_id;


    std::ostringstream message1;
    message1 << "Unit ID: " << m_unit_id << std::endl;
    ohd_log(STATUS_LEVEL_INFO, message1.str());

    std::ostringstream message2;
    std::string boot_type = is_air ? "Air" : "Ground";

    message2 << "Booting as: " << boot_type << std::endl;
    ohd_log(STATUS_LEVEL_INFO, message2.str());

    std::ostringstream message3;
    message3 << "Microservice sysid: " << sys_id << std::endl;
    ohd_log(STATUS_LEVEL_INFO, message3.str());


    return j;
}
