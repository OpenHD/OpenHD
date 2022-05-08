
#include <iostream>
#include <boost/asio.hpp>

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"

#include "OHDVideo.h"

int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    const auto profile=profile_from_manifest();
    const auto platform=platform_from_manifest();

    if(profile.is_air){
        OHDVideo ohdVideo(io_service,profile.is_air,profile.unit_id,platform.platform_type);
    }
    // TODO fix
    //sd_notify(0, "READY=1");
    std::cerr << "Video ready" << std::endl;

    // fake it for the moment
    boost::asio::io_service::work work(io_service);
    io_service.run();

    return 0;
}
