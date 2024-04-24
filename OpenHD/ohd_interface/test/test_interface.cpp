
#include <getopt.h>
#include <unistd.h>

#include <iostream>

#include "ohd_interface.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"

// Using test_interface:
// 1) run it as "air" on whatever device you want to use as the air unit
// 2) run it as "ground" on whatever device you want to use as the ground unit.
// 3) You can now pipe test data into the "air instance" via UDP (e.g. into the
// main stream video port) and listen with nc if any data arrives at the ground
// instance.

static const char optstr[] = "?s:b:";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {nullptr, 0, nullptr, 0},
};

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();
  bool is_air = false;
  {
    int c;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
      const char *tmp_optarg = optarg;
      switch (c) {
        case 'a':
          is_air = true;
          break;
        case '?':
        default:
          std::cout << "Usage: \n"
                    << "--air (-a) run as air, otherwise run as ground \n";
          exit(1);
      }
    }
  }
  std::cout << "Test_interface run as air:" << OHDUtil::yes_or_no(is_air)
            << "\n";

  const auto platform = OHDPlatform::instance();
  const OHDProfile profile{is_air, "0"};
  OHDInterface ohdInterface(profile);

  std::cout << "OHDInterface started\n";

  // run forever, OHDInterface runs in its own threads
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "XInterface\n";
  }
  std::cerr << "OHDInterface stopped\n";

  return 0;
}
