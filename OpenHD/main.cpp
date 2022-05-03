//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include "openhd-log.hpp"
#include "openhd-read-util.hpp"
#include "json.hpp"

#include <memory>
//#include <boost/asio/io_service.hpp>

int main(int argc, char *argv[]) {
    std::cout<<"OpenHD START\n";

    // Always needs to run first.
    //OHDSystem::runOnceOnStartup();

    //boost::asio::io_service io_service;

    const std::string unit_id=OHDReadUtil::get_unit_id();
    const bool is_air = OHDReadUtil::runs_on_air();


}