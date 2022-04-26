#include <iostream>
#include <iterator>
#include <exception>
#include <vector>

#include "boost/asio.hpp"
#include <boost/program_options.hpp>

#include "json.hpp"
using json = nlohmann::json;


#include "openhd-platform.hpp"
#include "openhd-settings.hpp"
#include "openhd-status.hpp"

#include "router.h"
#include "control.h"

int main(int argc, char *argv[]) {
    boost::asio::io_service io_service;

    try {
        boost::program_options::options_description desc("Options");

        desc.add_options()("help", "produce help message")
                          ("mode", boost::program_options::value<std::string>()->required(), "mode");

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            exit(0);
        }


        std::string mode = vm["mode"].as<std::string>();
        bool is_microservice = (mode == "microservice");


        std::string unit_id;
        bool is_air = false;

        try {
            std::ifstream f("/tmp/profile_manifest");
            nlohmann::json j;
            f >> j;

            is_air = j["is-air"];

            unit_id = j["unit-id"];
        } catch (std::exception &ex) {
            std::cerr << "Profile manifest processing failed: " << ex.what() << std::endl;
            ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
            exit(1);
        }



        PlatformType platform_type = PlatformTypeUnknown;
        
        try {
            std::ifstream f("/tmp/platform_manifest");
            nlohmann::json j;
            f >> j;
            platform_type = string_to_platform_type(j["platform"]);
        } catch (std::exception &ex) {
            std::cerr << "Platform manifest processing failed: " << ex.what() << std::endl;
            ohd_log(STATUS_LEVEL_EMERGENCY, "Platform manifest processing failed");
            exit(1);
        }

        auto router = new Router(io_service, platform_type, is_air, unit_id, is_microservice);

        auto control = new Control(router, io_service, is_microservice);


        io_service.run();

    } catch (std::exception &ex) {
        std::cerr << "Telemetry service has stopped unexpectedly: " << ex.what() << std::endl;

        ohd_log(STATUS_LEVEL_EMERGENCY, "Telemetry service has stopped unexpectedly");

        exit(1);
    }

    return 0;
}
