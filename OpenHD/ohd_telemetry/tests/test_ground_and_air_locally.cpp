//
// For testing, run the air and ground telemetry services side by side on the same machine locally.
//
#include <iostream>

#include "../src/OHDTelemetry.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include <thread>
#include <memory>

int main() {
  std::cout<<"start\n";
  std::unique_ptr<OHDTelemetry> ohdTelemGround;
  std::unique_ptr<OHDTelemetry> ohdTelemAir;
  {
	OHDProfile profile{false, "XX"};
	OHDPlatform platform{};
	ohdTelemGround = std::make_unique<OHDTelemetry>(platform, profile);
        auto example_comp=std::make_shared<openhd::DummyXSettingsComponent>();
        ohdTelemGround->add_settings_component(100,example_comp);
  }
  {
	OHDProfile profile{true, "XX"};
	OHDPlatform platform{PlatformType::PC};
	ohdTelemAir = std::make_unique<OHDTelemetry>(platform, profile);
        auto example_comp=std::make_shared<openhd::DummyXSettingsComponent>();
        ohdTelemAir->add_camera_component(0,example_comp);
  }
  while (true) {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::stringstream ss;
	ss<<"G and air debug:\n";
	ss<<ohdTelemGround->createDebug();
	ss<<ohdTelemAir->createDebug();
	std::cout<<ss.str();
  }
}

