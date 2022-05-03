//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include "openhd-log.hpp"
#include "openhd-read-util.hpp"
#include "ohd_system/inc/OHDSystem.h"
#include "ohd_interface/inc/OHDInterface.h"
//#include "ohd_video/inc/OHDVideo.h"

#include <memory>
#include <boost/asio/io_service.hpp>

int main(int argc, char *argv[]) {
    std::cout<<"OpenHD START\n";

    // Always needs to run first.
    OHDSystem::runOnceOnStartup();

    const std::string unit_id=OHDReadUtil::get_unit_id();
    const bool is_air = OHDReadUtil::runs_on_air();
    const auto platform_type=OHDReadUtil::get_platform_type();

    boost::asio::io_service io_service;
    auto ohdInterface=std::make_unique<OHDInterface>(io_service,is_air,unit_id);

    //auto ohdVideo=std::make_unique<OHDVideo>(io_service,is_air,unit_id,platform_type);

}