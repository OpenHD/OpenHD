
#include <camera_discovery.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "ohd_video_air.h"
#include "openhd_platform.hpp"
#include "openhd_platform_discover.hpp"
#include "openhd_profile.hpp"

int main(int argc, char *argv[]) {
  const auto platform=DPlatform::discover();

  auto tmp_opt=openhd::parse_video_format("1280x720@30");

  auto cameras=DCameras::discover(*platform);
  if(cameras.empty()){
    cameras.emplace_back(createDummyCamera());
  }
  OHDVideoAir ohdVideo(*platform,cameras, nullptr, nullptr);
  std::cout << "OHDVideo started\n";
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout<<"XOHDVid\n";
    std::cout<<ohdVideo.createDebug();
  }
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
