#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <systemd/sd-daemon.h>

#include "../inc/powermicroservice.h"
#include "openhd-platform.hpp"
#include "openhd-status.hpp"


//#include "record.h"
#include "json.hpp"


int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    PlatformType platform_type = PlatformTypeUnknown;
    bool is_air = false;
    std::string unit_id;

    try {
        std::ifstream f("/tmp/platform_manifest");
        nlohmann::json j;
        f >> j;

        platform_type = string_to_platform_type(j["platform"]);
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "Platform manifest processing failed");
        std::cerr << "Platform manifest error: " << ex.what() << std::endl;
    }

    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;

        is_air = j["is-air"];

        unit_id = j["unit-id"];
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        std::cerr << "EX: " << ex.what() << std::endl;
    }

    if (!is_air) {

    }


//    MavlinkControl mavlink_control(io_service, platform_type);
    //Record record(io_service);

    //mavlinkcontrol.armed.connect(&record.set_armed);
    //mavlinkcontrol.rc_channels.connect(&record.set_rc_channels);


//    try {
//        mavlink_control.setup();
//    } catch (std::exception &exception) {
//        std::cout << "Microservice connection failed: " << exception.what() << std::endl;
//        exit(1);
//    }

    PowerMicroservice * power_microservice;

    try {
        //record.setup();

//        if (is_air) {
        	power_microservice = new PowerMicroservice(io_service, platform_type, is_air, unit_id);
        	power_microservice->setup();
        	power_microservice->set_sysid(is_air ? 253 : 254);
//        }
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    sd_notify(0, "READY=1");

    io_service.run();

    return 0;
}
