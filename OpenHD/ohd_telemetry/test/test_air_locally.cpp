//
// For testing, run the air code such that it can communicate with a running
// ground instance (on localhost)
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
  std::unique_ptr<OHDTelemetry> air;
  {
    OHDProfile profile{true, "YY"};
    const auto platform = OHDPlatform::instance();
    air = std::make_unique<OHDTelemetry>(profile, true);
  }
  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << air->createDebug();
  }
}
