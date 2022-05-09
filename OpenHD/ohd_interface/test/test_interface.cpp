#include <iostream>
#include <boost/asio.hpp>

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

#include "OHDInterface.h"

int main(int argc, char *argv[]) {

    const auto profile=profile_from_manifest();
    const auto platform=platform_from_manifest();

    OHDInterface ohdInterface(profile.is_air,profile.unit_id);

    std::cerr<<"OHDInterface started\n";

    // run forever, OHDInterface runs in its own threads
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"XInterface\n";
    }

    std::cerr<<"OHDInterface stopped\n";

    return 0;
}
