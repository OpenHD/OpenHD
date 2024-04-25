//
// For testing, run the ground code such that it can communicate with QOpenHD,
// but there is no air unit.
//
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

#include "../src/OHDTelemetry.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_imp.h"
#include "openhd_spdlog_include.h"

int main() {
  std::cout << "start\n";
  std::unique_ptr<OHDTelemetry> ground;
  // std::this_thread::sleep_for(std::chrono::seconds(10));
  {
    OHDProfile profile{false, "XX"};
    const auto platform = OHDPlatform::instance();
    ground = std::make_unique<OHDTelemetry>(profile);
    // MAV_COMP_ID_ONBOARD_COMPUTER2=192
    ground->add_settings_generic(
        openhd::testing::create_dummy_ground_settings());
    // ground->add_external_ground_station_ip("192.168.237.22","192.168.237.55");
  }
  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << ground->createDebug();
  }
}