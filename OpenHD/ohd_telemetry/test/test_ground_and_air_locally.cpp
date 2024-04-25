//
// For testing, run the air and ground telemetry services side by side on the
// same machine locally.
//
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

#include "../src/OHDTelemetry.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_spdlog_include.h"

int main() {
  std::cout << "start\n";
  std::unique_ptr<OHDTelemetry> ohdTelemGround;
  std::unique_ptr<OHDTelemetry> ohdTelemAir;
  {
    OHDProfile profile{false, "XX"};
    const auto platform = OHDPlatform::instance();
    ohdTelemGround = std::make_unique<OHDTelemetry>(profile);
    ohdTelemGround->add_settings_generic(
        openhd::testing::create_dummy_ground_settings());
  }
  {
    OHDProfile profile{true, "XX"};
    const auto platform = OHDPlatform::instance();
    ohdTelemAir = std::make_unique<OHDTelemetry>(profile);
    ohdTelemAir->add_settings_generic(
        openhd::testing::create_dummy_camera_settings());
  }
  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::stringstream ss;
    ss << "G and air debug:\n";
    ss << ohdTelemGround->createDebug();
    ss << ohdTelemAir->createDebug();
    std::cout << ss.str();
  }
}
