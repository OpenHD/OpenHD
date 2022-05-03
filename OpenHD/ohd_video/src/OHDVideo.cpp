//
// Created by consti10 on 03.05.22.
//

#include "OHDVideo.h"

OHDVideo::OHDVideo(boost::asio::io_service &io_service, bool is_air, std::string unit_id,PlatformType platform_type):io_service(io_service),is_air(is_air),unit_id(unit_id),platform_type(platform_type) {
    try {
        if (is_air) {
            // TDOD: re-write this one
            camera_microservice =std::make_unique<CameraMicroservice>(io_service, platform_type, is_air, unit_id);
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
