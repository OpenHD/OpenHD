//
// For testing, run the air code such that it can communicate with a running ground instance (on localhost)
//
#include <iostream>

#include "../src/OHDTelemetry.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include <thread>
#include <memory>

int main() {
  std::cout<< "start\n";
  std::unique_ptr<OHDTelemetry> air;
  {
	OHDProfile profile{true, "YY"};
	OHDPlatform platform{};
	platform.platform_type = PlatformTypePC;
	air = std::make_unique<OHDTelemetry>(platform, profile,true);
  }
  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::cout<<air->createDebug();
  }
}

