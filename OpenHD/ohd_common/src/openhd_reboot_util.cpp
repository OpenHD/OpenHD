//
// Created by consti10 on 14.02.23.
//

#include "openhd_reboot_util.h"

#include <thread>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_async.h"

/**
 * Systemctl is the nicer way, but sometimes has issues.
 */
void openhd::reboot::systemctl_shutdown() {
  OHDUtil::run_command("systemctl", {"start", "poweroff.target"}, true);
}

void openhd::reboot::systemctl_reboot() {
  OHDUtil::run_command("systemctl", {"start", "reboot.target"}, true);
}

static void command_reboot() { OHDUtil::run_command("reboot", {}, true); }
static void command_shutdown() { OHDUtil::run_command("shutdown", {}, true); }

void openhd::reboot::systemctl_power(bool shutdownOnly) {
  if (shutdownOnly) {
    // Zero3w seems to be bugged
    if (OHDPlatform::instance().platform_type ==
        X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) {
      command_shutdown();
    } else {
      systemctl_shutdown();
    }
  } else {
    if (OHDPlatform::instance().platform_type ==
        X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) {
      command_reboot();
    } else {
      systemctl_reboot();
    }
  }
}

void openhd::reboot::handle_power_command_async(std::chrono::milliseconds delay,
                                                bool shutdownOnly) {
  const std::string tag = shutdownOnly ? "SHUTDOWN" : "REBOOT";
  AsyncHandle::instance().execute_async(tag, [delay, shutdownOnly] {
    std::this_thread::sleep_for(delay);
    systemctl_power(shutdownOnly);
  });
}
