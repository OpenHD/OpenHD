//
// Created by consti10 on 02.05.22.
//

#include "OHDDiscovery.h"

#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include "DPlatform.h"
#include "DCameras.h"
//#include "DEthernetCards.h"
#include "DWifiCards.h"
#include "DProfile.h"

#include "json.hpp"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-settings.hpp"
#include "openhd-wifi.hpp"
#include "openhd-camera.hpp"

void OHDDiscovery::runOnceOnStartup(bool forceAir){
    std::cout<<"OHDSystem::runOnceOnStartup()\n";
    try {
        DPlatform platform;
        platform.discover();
        auto platform_manifest = platform.generate_manifest();
        std::ofstream _p(PLATFORM_MANIFEST_FILENAME);
        _p << platform_manifest.dump(4);
        _p.close();

        DCameras cameras(platform.platform_type(), platform.board_type(), platform.carrier_type());
        cameras.discover();
        auto camera_manifest = cameras.generate_manifest();
        std::ofstream _c(CAMERA_MANIFEST_FILENAME);
        _c << camera_manifest.dump(4);
        _c.close();


        DWifiCards wifi(platform.platform_type(), platform.board_type(), platform.carrier_type());
        wifi.discover();
        auto wifi_manifest = wifi.generate_manifest();
        std::ofstream _w(WIFI_MANIFEST_FILENAME);
        _w << wifi_manifest.dump(4);
        _w.close();

        /*DEthernetCards ethernet(platform.platform_type(), platform.board_type(), platform.carrier_type());
        ethernet.discover();
        auto ethernet_manifest = ethernet.generate_manifest();
        std::ofstream _t("/tmp/ethernet_manifest");
        _t << ethernet_manifest.dump(4);
        _t.close();*/

        // When we write the profile we need to reason weather this is an air or ground pi.
        const int camera_count = cameras.count();
        bool is_air=camera_count > 0;
        if(forceAir){
            is_air=true;
        }
        DProfile profile(is_air);
        profile.discover();
        auto profile_manifest = profile.generate_manifest();
        std::ofstream _pr(PROFILE_MANIFEST_FILENAME);
        _pr << profile_manifest.dump(4);
        _pr.close();

        // Note: Here stephen wrote all the small sub-manifests into one big manifest.
        // In my opinion, there is an apparent issue with that: The data is suddenly duplicated,
        // and one cannot know what is actually then read by the services running after -
        // The sub-files or the one big file.
        // Since sub-files promote separation, there is only sub-files now.
        std::cout<<"OHDSystem::done\n";
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }
}
