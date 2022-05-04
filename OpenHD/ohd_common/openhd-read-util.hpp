//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_OPENHD_PLATFORM_UTIL_H
#define OPENHD_OPENHD_PLATFORM_UTIL_H

#include "json.hpp"
#include "openhd-log.hpp"
#include "openhd-platform.hpp"
#include <fstream>
#include <sstream>

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
            std::stringstream ss;
            ss<<"Profile manifest processing failed (unit_id)"<<ex.what()<<"\n";
            ohd_log(STATUS_LEVEL_EMERGENCY, ss.str());
        }
        return unit_id;
    }
    static PlatformType get_platform_type(){
        PlatformType platform_type = PlatformTypeUnknown;
        try {
            std::ifstream f("/tmp/platform_manifest");
            nlohmann::json j;
            f >> j;
            platform_type = string_to_platform_type(j["platform"]);
        } catch (std::exception &ex) {
            // don't do anything, but send an error message to the user through the status service
            std::stringstream ss;
            ss<<"Platform manifest processing failed"<<ex.what()<<"\n";
            ohd_log(STATUS_LEVEL_EMERGENCY, ss.str());
        }
        return platform_type;
    }
}

#endif //OPENHD_OPENHD_PLATFORM_UTIL_H
