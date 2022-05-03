//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include "ohd_interface/inc/OHDInterface.h"
#include "ohd_system/inc/OHDSystem.h"
#include "ohd_video/inc/OHDVideo.h"
#include "openhd-log.hpp"
#include "json.hpp"

#include <memory>

int main(int argc, char *argv[]) {
    std::cout<<"OpenHD START\n";

    // Always needs to run first.
    OHDSystem::runOnceOnStartup();

    boost::asio::io_service io_service;

    std::string unit_id;
    bool is_air = false;

    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;

        is_air = j["is-air"];

        unit_id = j["unit-id"];
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
    }

}