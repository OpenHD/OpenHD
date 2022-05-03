
#include <iostream>
#include <boost/asio.hpp>

#include "openhd-read-util.hpp"
#include "OHDVideo.h"


int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    const std::string unit_id=OHDReadUtil::get_unit_id();
    const bool is_air = OHDReadUtil::runs_on_air();
    const auto platform_type=OHDReadUtil::get_platform_type();
    if(is_air){
        OHDVideo ohdVideo(io_service,is_air,unit_id,platform_type);
    }
    // TODO fix
    //sd_notify(0, "READY=1");
    std::cerr << "Video ready" << std::endl;

    // fake it for the moment
    boost::asio::io_service::work work(io_service);
    io_service.run();

    return 0;
}
