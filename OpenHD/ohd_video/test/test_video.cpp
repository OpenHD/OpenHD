
#include <camera_discovery.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "ohd_video_air.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_link.hpp"
#include "openhd_util.h"

// Can be used to test / validate a camera implementation.
//
int main(int argc, char *argv[]) {
  // We need root to read / write camera settings.
  OHDUtil::terminate_if_not_root();
  const auto platform=DPlatform::discover();
  auto cameras=DCameras::discover(*platform);
  if(cameras.empty()){
    cameras.emplace_back(createDummyCamera());
  }
  auto debug_link=std::make_shared<DummyDebugLink>();
  OHDVideoAir ohdVideo(*platform,cameras, nullptr, debug_link);
  std::cout << "OHDVideo started\n";
  OHDUtil::keep_alive_until_sigterm();
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
