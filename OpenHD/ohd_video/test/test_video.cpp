
#include <iostream>
#include <thread>
#include <chrono>
#include <DCameras.h>

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-platform-discover.hpp"

#include "OHDVideo.h"

int main(int argc, char *argv[]) {
  const auto platform=DPlatform::discover();
  auto libcameraProvider = std::make_shared<LibcameraProvider>();

  auto cameras=DCameras::discover(*platform, libcameraProvider);
  if(cameras.empty()){
    cameras.emplace_back(createDummyCamera());
  }
  OHDVideo ohdVideo(*platform,cameras);
  std::cout << "OHDVideo started\n";
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout<<"XOHDVid\n";
    std::cout<<ohdVideo.createDebug();
  }
  std::cerr << "OHDVideo stopped\n";
  return 0;
}
