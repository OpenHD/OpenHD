#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include <systemd/sd-daemon.h>

#include "../inc/statusmicroservice.h"
#include "openhd-platform.hpp"
#include "openhd-status.hpp"


//#include "record.h"
#include "json.hpp"


int main(int argc, char *argv[]) {

	boost::asio::io_service io_service;

	PlatformType platform_type = PlatformTypeUnknown;
	bool is_air = false;
	std::string unit_id;

	try {
		std::ifstream f("/tmp/platform_manifest");
		nlohmann::json j;
		f >> j;

		platform_type = string_to_platform_type(j["platform"]);
	} catch (std::exception &ex) {
		// don't do anything, but send an error message to the user through the status service
		status_message(STATUS_LEVEL_EMERGENCY, "Platform manifest processing failed");
		std::cerr << "Platform manifest error: " << ex.what() << std::endl;
	}

	try {
		std::ifstream f("/tmp/profile_manifest");
		nlohmann::json j;
		f >> j;

		is_air = j["is-air"];

		unit_id = j["unit-id"];
	} catch (std::exception &ex) {
		// don't do anything, but send an error message to the user through the status service
		status_message(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
		std::cerr << "EX: " << ex.what() << std::endl;
	}

	if (!is_air) {
		std::vector<std::string> openhdboot { 
            "start", "openhdboot"
        };

        boost::process::child c_systemctl_openhdboot(boost::process::search_path("systemctl"), boot);
        c_systemctl_openhdboot.wait();

		std::vector<std::string> qopenhd { 
            "start", "qopenhd"
        };

        boost::process::child c_systemctl_qopenhd(boost::process::search_path("systemctl"), qopenhd);
        c_systemctl_qopenhd.wait();

	}


	StatusMicroservice * status_microservice;

	try {

		status_microservice = new StatusMicroservice(io_service, platform_type, is_air, unit_id);
		status_microservice->setup();
		status_microservice->set_sysid(is_air ? 253 : 254);
	} catch (std::exception &ex) {
		std::cerr << "Error: " << ex.what() << std::endl;
		exit(1);
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
		exit(1);
	}

	sd_notify(0, "READY=1");

	io_service.run();

	return 0;
}
