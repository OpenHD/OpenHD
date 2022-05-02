//
// Created by consti10 on 03.05.22.
//

#include "OHDVideo.h"

OHDVideo::OHDVideo(boost::asio::io_service &io_service, bool is_air, std::string unit_id,PlatformType platform_type):io_service(io_service),is_air(is_air),unit_id(unit_id),platform_type(platform_type) {

    // copy paste from main
    // TODO un-spaghetti this crap
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
}
