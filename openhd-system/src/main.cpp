#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include "platform.h"
#include "cameras.h"
#include "wifi.h"
#include "profile.h"

#include "json.hpp"


int main(int argc, char *argv[]) {


    try {
        Platform platform;
        platform.discover();
        auto platform_manifest = platform.generate_manifest();
        std::ofstream _p("/tmp/platform_manifest");
        _p << platform_manifest.dump(4);
        _p.close();


        Cameras cameras(platform.platform_type(), platform.board_type(), platform.carrier_type());
        cameras.discover();
        auto camera_manifest = cameras.generate_manifest();
        std::ofstream _c("/tmp/camera_manifest");
        _c << camera_manifest.dump(4);
        _c.close();


        WiFi wifi(platform.platform_type(), platform.board_type(), platform.carrier_type(), platform.wifi_hotspot_type());
        wifi.discover();
        auto wifi_manifest = wifi.generate_manifest();
        std::ofstream _w("/tmp/wifi_manifest");
        _w << wifi_manifest.dump(4);
        _w.close();

        Profile profile(platform.platform_type(), platform.board_type(), platform.carrier_type());
        profile.discover();
        auto profile_manifest = profile.generate_manifest();
        std::ofstream _pr("/tmp/profile_manifest");
        _pr << profile_manifest.dump(4);
        _pr.close();

    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    return 0;
}
