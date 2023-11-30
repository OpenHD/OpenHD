
#include <camera_discovery.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "ohd_video_air.h"
#include "openhd_platform.h"
#include "openhd_profile.h"

int main(int argc, char *argv[]) {
  const auto platform=DPlatform::discover();

  auto tmp_opt=openhd::parse_video_format("1280x720@30");

  auto cameras=DCameras::discover(*platform);
  if(cameras.empty()){
    cameras.emplace_back(createDummyCamera());
  }
  OHDVideoAir ohdVideo(*platform,cameras, nullptr, nullptr);
  std::cout << "OHDVideo started\n";
  OHDUtil::keep_alive_until_sigterm();
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
