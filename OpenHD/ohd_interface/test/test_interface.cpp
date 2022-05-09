#include <iostream>
#include <boost/asio.hpp>

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

#include "OHDInterface.h"

int main(int argc, char *argv[]) {

    const auto profile=profile_from_manifest();
    const auto platform=platform_from_manifest();

    OHDInterface ohdInterface(profile.is_air,profile.unit_id);

    while(true){

    }

    std::cerr << "OpenHD Interface service exiting, this should not happen" << std::endl;

    return 0;
}
