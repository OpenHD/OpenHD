//
// For testing, run the ground code such that it can communicate with QOpenHD, but there is no air unit.
//
#include <iostream>

#include "../src/OHDTelemetry.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-platform-discover.hpp"
#include "mavlink_settings/ISettingsComponent.h"
#include <thread>
#include <memory>

int main() {
  std::cout<< "start\n";
  std::unique_ptr<OHDTelemetry> ground;
  //std::this_thread::sleep_for(std::chrono::seconds(10));
  {
	OHDProfile profile{false, "XX"};
	const auto platform=DPlatform::discover();
	ground = std::make_unique<OHDTelemetry>(*platform, profile);
	// MAV_COMP_ID_ONBOARD_COMPUTER2=192
	ground->add_settings_generic(openhd::testing::create_dummy_ground_settings());
	//ground->add_external_ground_station_ip("192.168.237.22","192.168.237.55");
  }
  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<ground->createDebug();
  }
}