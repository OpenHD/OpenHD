//
// For testing, run the air code such that it can communicate with a running ground instance (on localhost)
//
#include <iostream>

#include "../src/OHDTelemetry.h"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-platform-discover.hpp"
#include <thread>
#include <memory>

int main() {
  std::cout<< "start\n";
  std::unique_ptr<OHDTelemetry> air;
  {
	OHDProfile profile{true, "YY"};
        const auto platform=DPlatform::discover();
	air = std::make_unique<OHDTelemetry>(*platform, profile,nullptr,true);
  }
  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<air->createDebug();
  }
}

