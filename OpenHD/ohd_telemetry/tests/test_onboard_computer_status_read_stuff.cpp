// For the onboard computer status we read a lot of stuff,
// This is for testing these functionalities

#include <memory>

#include "../src/internal/LogCustomOHDMessages.hpp"
#include "../src/internal/OnboardComputerStatusProvider.h"
#include "openhd_platform.h"

int main() {

  const auto platform=DPlatform::discover();
  const auto provider=std::make_unique<OnboardComputerStatusProvider>(*platform);

  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
    auto tmp=provider->get_current_status();
    LogCustomOHDMessages::logOnboardComputerStatus(tmp);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}