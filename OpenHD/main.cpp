//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include "openhd-log.hpp"
#include "openhd-read-util.hpp"
#include "ohd_system/inc/OHDSystem.h"
#include "ohd_interface/inc/OHDInterface.h"
#include "ohd_video/inc/OHDVideo.h"
#include "ohd_telemetry/src/GroundTelemetry.h"
#include "ohd_telemetry/src/AirTelemetry.h"

#include <memory>
#include <boost/asio/io_service.hpp>

//TODO fix the cmake crap and then we can build a single executable.

int main(int argc, char *argv[]) {
    std::cout<<"OpenHD START\n";

    // Always needs to run first.
    /*OHDSystem::runOnceOnStartup();

    // Now this is kinda stupid - we write json's during the discovery, then we read them back in
    // merged stupidly with some fragments that resemble settings.
    const std::string unit_id=OHDReadUtil::get_unit_id();
    const bool is_air = OHDReadUtil::runs_on_air();
    const auto platform_type=

    // These 2 get all the hardware info from json
    auto ohdInterface=std::make_unique<OHDInterface>(is_air,unit_id);
    boost::asio::io_service io_service;

    auto ohdVideo=std::make_unique<OHDVideo>(io_service,is_air,unit_id,platform_type);

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);
    io_service.run();*/

}