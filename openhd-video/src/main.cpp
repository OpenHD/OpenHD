#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <systemd/sd-daemon.h>

#include "openhd-platform.hpp"
#include "openhd-status.hpp"

#include "control.h"
//#include "mavlinkcontrol.h"
#include "hotspot.h"
//#include "record.h"
#include "cameramicroservice.h"


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
        status_message(STATUS_LEVEL_EMERGENCY, "Platform manifest processing failed");
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
        status_message(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        std::cerr << "EX: " << ex.what() << std::endl;
    }

    Control *control;
    Hotspot *hotspot1;
    Hotspot *hotspot2;
    Hotspot *hotspot3;
    Hotspot *hotspot4;

    if (!is_air) {
        control = new Control(io_service);

        try {
            control->setup();
        } catch (std::exception &exception) {
            std::cout << "Control setup failed: " << exception.what() << std::endl;
            exit(1);
        }
        
        hotspot1 = new Hotspot(io_service, 5620);
        hotspot2 = new Hotspot(io_service, 5621);
        hotspot3 = new Hotspot(io_service, 5622);
        hotspot4 = new Hotspot(io_service, 5623);

        control->command_received.connect(boost::bind(&Hotspot::command_received, hotspot1, _1));
        control->command_received.connect(boost::bind(&Hotspot::command_received, hotspot2, _1));
        control->command_received.connect(boost::bind(&Hotspot::command_received, hotspot3, _1));
        control->command_received.connect(boost::bind(&Hotspot::command_received, hotspot4, _1));
    }


    //MavlinkControl mavlink_control(io_service, platform_type);
    //Record record(io_service);

    //mavlinkcontrol.armed.connect(&record.set_armed);
    //mavlinkcontrol.rc_channels.connect(&record.set_rc_channels);


    try {
        //mavlink_control.setup();
    } catch (std::exception &exception) {
        std::cout << "Microservice connection failed: " << exception.what() << std::endl;
        exit(1);
    }

    CameraMicroservice * camera_microservice;

    try {
        //record.setup();

        if (is_air) {
            camera_microservice = new CameraMicroservice(io_service, platform_type, is_air, unit_id);
            camera_microservice->setup();
        }
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
