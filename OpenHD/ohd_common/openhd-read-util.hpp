//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_OPENHD_PLATFORM_UTIL_H
#define OPENHD_OPENHD_PLATFORM_UTIL_H

#include "json.hpp"
using json = nlohmann::json;
#include "openhd-log.hpp"

// Generally they need to be run after openhd-system
namespace OHDReadUtil{
    // Determine at run time if we are running on air or ground pi
    // !!!!! Needs to be used AFTER openhd-system has written all the config stuff !!!!
    static bool runs_on_air(){
        bool is_air = false;
        try {
            std::ifstream f("/tmp/profile_manifest");
            nlohmann::json j;
            f >> j;
            is_air = j["is-air"];
        } catch (std::exception &ex) {
            std::cerr << "Profile manifest processing failed: " << ex.what() << std::endl;
            ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed (is_air)");
            exit(1);
        }
        return is_air;
    }

    // No idea what this one is for
    // TODO document or get rid off
    static std::string get_unit_id(){
        std::string unit_id;
        try {
            std::ifstream f("/tmp/profile_manifest");
            nlohmann::json j;
            f >> j;
            unit_id = j["unit-id"];
        } catch (std::exception &ex) {
            // don't do anything, but send an error message to the user through the status service
            ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed (unit_id)");
        }
        return unit_id;
    }
}

#endif //OPENHD_OPENHD_PLATFORM_UTIL_H
