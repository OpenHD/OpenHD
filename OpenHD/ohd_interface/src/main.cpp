#include <iostream>
#include <boost/asio.hpp>

#include "openhd-read-util.hpp"
#include "OHDInterface.h"

int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    const std::string unit_id=OHDReadUtil::get_unit_id();
    const bool is_air = OHDReadUtil::runs_on_air();

    OHDInterface ohdInterface(is_air,unit_id);

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);

    // TODO !!! service notify
    //sd_notify(0, "READY=1");

    io_service.run();

    std::cerr << "OpenHD Interface service exiting, this should not happen" << std::endl;

    return 0;
}
