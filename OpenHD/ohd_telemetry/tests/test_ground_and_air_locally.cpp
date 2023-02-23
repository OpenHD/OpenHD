//
// For testing, run the air and ground telemetry services side by side on the same machine locally.
//
#include <iostream>
#include <memory>
#include <thread>
#include <csignal>

#include "../src/OHDTelemetry.h"
#include "openhd_platform.h"
#include "openhd_profile.h"

int main() {
  std::cout<<"start\n";
  std::unique_ptr<OHDTelemetry> ohdTelemGround;
  std::unique_ptr<OHDTelemetry> ohdTelemAir;
  {
    OHDProfile profile{false, "XX"};
    const auto platform=DPlatform::discover();
    ohdTelemGround = std::make_unique<OHDTelemetry>(*platform, profile);
    ohdTelemGround->add_settings_generic(openhd::testing::create_dummy_ground_settings());
  }
  {
    OHDProfile profile{true, "XX"};
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

