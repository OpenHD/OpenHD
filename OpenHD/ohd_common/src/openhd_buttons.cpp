//
// Created by consti10 on 02.02.24.
//

#include "openhd_buttons.h"

#include <thread>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"

namespace openhd::rpi {

// RPI GPIO26: Used as a reset button

// After configure: Connect gpio26 to ground -> reports 0
// Do not connect gpio26 to ground -> reports 1
static void gpio26_configure() {
  OHDUtil::run_command("raspi-gpio", {"set", "26", "ip", "pu"});
};

static bool gpio26_user_wants_reset_frequencies() {
  const auto res_opt = OHDUtil::run_command_out("raspi-gpio get 26");
  if (res_opt) {
    const auto res = res_opt.value();
    if (OHDUtil::contains(res, "GPIO 26: level=0 fsel=0 func=INPUT pull=UP")) {
      openhd::log::get_default()->info(
          "GPIO26 pull UP and level=0, user_wants_reset_frequencies");
      return true;
    }
  }
  return false;
};

}  // namespace openhd::rpi
openhd::ButtonManager& openhd::ButtonManager::instance() {
  static ButtonManager instance;
  return instance;
}

bool openhd::ButtonManager::user_wants_reset_openhd_core() {
  // Right now only supported on rpi
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::gpio26_configure();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto res_opt = OHDUtil::run_command_out("raspi-gpio get 26");
    if (res_opt) {
      const auto res = res_opt.value();
      if (OHDUtil::contains(res,
                            "GPIO 26: level=0 fsel=0 func=INPUT pull=UP")) {
        openhd::log::get_default()->info(
            "GPIO26 pull UP and level=0, user_wants_reset_frequencies");
        return true;
      }
    }
  }
  return false;
}
