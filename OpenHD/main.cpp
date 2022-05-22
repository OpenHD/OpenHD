//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include <memory>
#include <sstream>

#include "ohd_common/openhd-platform.hpp"
#include "ohd_common/openhd-profile.hpp"

#include <OHDDiscovery.h>
#include <OHDInterface.h>
#include <OHDVideo.h>
#include <OHDTelemetry.hpp>

//TODO fix the cmake crap and then we can build a single executable.
static const char optstr[] = "?:da";
static const struct option long_options[] = {
	{"skip_discovery", no_argument, nullptr, 'd'},
	{"force_air", no_argument, nullptr, 'a'},
	{NULL, 0, nullptr, 0},
};

struct OHDRunOptions {
  bool skip_discovery = false;
  bool force_air = false;
};

int main(int argc, char *argv[]) {
  OHDRunOptions options{};
  // parse some arguments usefully for debugging
  int c;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
	const char *tmp_optarg = optarg;
	switch (c) {
	  case 'd':options.skip_discovery = true;
		break;
	  case 'a':options.force_air = true;
		break;
	  case '?':
	  default:
		std::cout << "Usage: --skip_detection [Skip detection step, usefully for changing things in json manually] " <<
				  "force_air [Force to boot as air pi, even when no camera is detected] " <<
				  "\n";
		return 0;
	}
  }
  std::cout << "OpenHD START with " <<
			"skip_discovery:" << (options.skip_discovery ? "Y" : "N") <<
			"force_air:" << (options.force_air ? "Y" : "N") <<
			"\n";

  try {
	if (!options.skip_discovery) {
	  // Always needs to run first.
	  OHDDiscovery::runOnceOnStartup(options.force_air);
	}

	// Now this is kinda stupid - we write json's during the discovery, then we read them back in
	// merged stupidly with some fragments that resemble settings.
	// Note that interface, telemetry and video might also read the or update the jsons
	const auto platform = platform_from_manifest();
	const auto profile = profile_from_manifest();

	// First start ohdInterface, which does wifibroadcast and more
	auto ohdInterface = std::make_unique<OHDInterface>(profile);

	// then we can start telemetry, which uses OHDInterface for wfb tx/rx (udp)
	auto telemetry = std::make_unique<OHDTelemetry>(platform, profile);

	// and start ohdVideo if we are on the air pi
	std::unique_ptr<OHDVideo> ohdVideo;
	if (profile.is_air) {
	  ohdVideo = std::make_unique<OHDVideo>(platform, profile);
	}

	std::cout << "All OpenHD modules running\n";

	// run forever, everything has its own threads. Note that the only way to break out basically
	// is when one of the modules encounters an exception.
	while (true) {
	  std::this_thread::sleep_for(std::chrono::seconds(2));
	  // To make sure this is all tightly packed together, we write it to a stringstream first
	  // and then to stdout in one big chunk. Otherwise, some other debug output might stand in between the OpenHD
	  // state debug chunk.
	  std::cout<< "---------------------------------OpenHD-state debug begin ---------------------------------\n";
	  std::cout<<ohdInterface->createDebug();
	  telemetry->debug();
	  if(ohdVideo){
		ohdVideo->restartIfStopped();
		std::cout<<ohdVideo->createDebug();
	  }
	  std::cout << "---------------------------------OpenHD-state debug   end ---------------------------------\n";
	}
  } catch (std::exception &ex) {
	std::cerr << "Error: " << ex.what() << std::endl;
	exit(1);
  } catch (...) {
	std::cerr << "Unknown exception occurred" << std::endl;
	exit(1);
  }
}
