#include <iostream>
#include <boost/asio.hpp>

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

#include "OHDInterface.h"

int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    const auto profile=profile_from_manifest();
    const auto platform=platform_from_manifest();

    OHDInterface ohdInterface(profile.is_air,profile.unit_id);

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);

    io_service.run();

    std::cerr << "OpenHD Interface service exiting, this should not happen" << std::endl;

    return 0;
}
