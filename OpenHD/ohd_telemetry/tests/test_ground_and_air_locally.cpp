//
// For testing, run the air and ground telemetry services side by side on the same machine locally.
//
#include "../src/OHDTelemetry.h"
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform-discover.hpp"

#include <iostream>
#include <memory>
#include <thread>

int main() {
  std::cout<<"start\n";
  std::unique_ptr<OHDTelemetry> ohdTelemGround;
  std::unique_ptr<OHDTelemetry> ohdTelemAir;
  {
	OHDProfile profile{false, "XX"};
	//OHDPlatform platform{PlatformType::PC};
	const auto platform=DPlatform::discover();
	ohdTelemGround = std::make_unique<OHDTelemetry>(*platform, profile);

	// MAV_COMP_ID_ONBOARD_COMPUTER2=192
	ohdTelemGround->add_settings_generic(openhd::testing::create_dummy_ground_settings());
  }
  {
	OHDProfile profile{true, "XX"};
	//OHDPlatform platform{PlatformType::PC};
	const auto platform=DPlatform::discover();
	ohdTelemAir = std::make_unique<OHDTelemetry>(*platform, profile);
	ohdTelemAir->add_settings_generic(openhd::testing::create_dummy_camera_settings());
  }
  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::stringstream ss;
	ss<<"G and air debug:\n";
	ss<<ohdTelemGround->createDebug();
	ss<<ohdTelemAir->createDebug();
	std::cout<<ss.str();
  }
}

