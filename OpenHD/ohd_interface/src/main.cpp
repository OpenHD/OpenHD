#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

//#include <systemd/sd-daemon.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
//#include <boost/bind.hpp>


#include "openhd-platform.hpp"
#include "openhd-log.hpp"


#include "streams.h"
#include "wifi.h"
#include "ethernet.h"

#include "json.hpp"

#include "OHDInterface.h"

int main(int argc, char *argv[]) {

    boost::asio::io_service io_service;

    std::string unit_id;
    bool is_air = false;

    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;

        is_air = j["is-air"];

        unit_id = j["unit-id"];
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
    }

    OHDInterface ohdInterface(io_service,is_air,unit_id);

    // fake it for the moment so the service doesn't exit, won't be needed once the microservice channel is wired in
    boost::asio::io_service::work work(io_service);

    // TODO !!! service notify
    //sd_notify(0, "READY=1");

    io_service.run();

    std::cerr << "OpenHD Interface service exiting, this should not happen" << std::endl;

    return 0;
}
