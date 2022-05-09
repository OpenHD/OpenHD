//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include <memory>
#include <boost/asio/io_service.hpp>

#include "ohd_common/openhd-platform.hpp"
#include "ohd_common/openhd-profile.hpp"

#include <OHDSystem.h>
#include <OHDInterface.h>
#include <OHDVideo.h>
#include <AirTelemetry.h>
#include <GroundTelemetry.h>

//TODO fix the cmake crap and then we can build a single executable.

int main(int argc, char *argv[]) {
    std::cout<<"OpenHD START\n";

    // Always needs to run first.
    OHDSystem::runOnceOnStartup();

    // Now this is kinda stupid - we write json's during the discovery, then we read them back in
    // merged stupidly with some fragments that resemble settings.
    const auto platform=platform_from_manifest();
    const auto profile=profile_from_manifest();


    // First start ohdInterface, which does wifibroadcast and more
    auto ohdInterface=std::make_unique<OHDInterface>(profile.is_air,profile.unit_id);
    boost::asio::io_service io_service;

    // then we can start telemetry, which uses OHDInterface for wfb tx/rx (udp)
    if(profile.is_air){
        auto telemetry=std::make_unique<AirTelemetry>();
    }else{
        auto telemetry=std::make_unique<GroundTelemetry>();
    }

    // and start ohdVideo if we are on the air pi
    if(profile.is_air){
        auto ohdVideo=std::make_unique<OHDVideo>(io_service,profile.is_air,profile.unit_id,platform.platform_type);
    }

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);
    io_service.run();

}