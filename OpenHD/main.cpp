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
#include <OHDTelemetry.hpp>

//TODO fix the cmake crap and then we can build a single executable.
static const char optstr[] = "?:da";
static const struct option long_options[] = {
        {"skip_detection", no_argument, NULL, 'd'},
        {"force_air", no_argument, NULL, 'a'},
        {NULL, 0, NULL, 0},
};

struct OHDRunOptions{
    bool skip_detection=false;
    bool force_air=false;
};

int main(int argc, char *argv[]) {
    OHDRunOptions options;
    // parse some arguments usefully for debugging
    int c;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
        const char *tmp_optarg = optarg;
        switch (c) {
            case 'd':
                options.skip_detection=true;
                break;
            case 'a':
                options.force_air=true;
                break;
            case '?':
            default:
                std::cout<<"Usage: --skip_detection [Skip detection step, usefully for changing things in json manually] "<<
                "force_air [Force to boot as air pi, even when no camera is detected] "<<
                "\n";
                return 0;
        }
    }
    std::cout<<"OpenHD START with "<<
             "skip_detection:"<<(options.skip_detection ? "Y": "N")<<
             "force_air:"<<(options.force_air ? "Y": "N")<<
             "\n";

    try{
        if(!options.skip_detection){
            // Always needs to run first.
            OHDSystem::runOnceOnStartup();
        }

        // Now this is kinda stupid - we write json's during the discovery, then we read them back in
        // merged stupidly with some fragments that resemble settings.
        // Note that interface, telemetry and video might also read the or update the jsons
        const auto platform=platform_from_manifest();
        const auto profile=profile_from_manifest();

        // First start ohdInterface, which does wifibroadcast and more
        auto ohdInterface=std::make_unique<OHDInterface>(profile.is_air,profile.unit_id);

        // then we can start telemetry, which uses OHDInterface for wfb tx/rx (udp)
        auto telemetry=std::make_unique<OHDTelemetry>(platform,profile);

        // and start ohdVideo if we are on the air pi
        if(profile.is_air){
            auto ohdVideo=std::make_unique<OHDVideo>(profile.is_air,profile.unit_id,platform.platform_type);
        }

        std::cout<<"All OpenHD modules running\n";

        // run forever, everything has its own threads. Note that the only way to break out basically
        // is when one of the modules encounters an exception.
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"OpenHD\n";
        }
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }
}
