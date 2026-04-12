/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

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
  OHDUtil::run_command("systemctl", {"--no-wall", "poweroff"}, true);
}

void openhd::reboot::systemctl_reboot() {
  OHDUtil::run_command("systemctl", {"--no-wall", "reboot"}, true);
}

static void systemctl_force_shutdown() {
  OHDUtil::run_command("systemctl",
                       {"--no-wall", "--force", "--force", "poweroff"}, true);
}

static void systemctl_force_reboot() {
  OHDUtil::run_command("systemctl",
                       {"--no-wall", "--force", "--force", "reboot"}, true);
}

static void command_reboot() { OHDUtil::run_command("reboot", {"-f"}, true); }
static void command_shutdown() {
  OHDUtil::run_command("poweroff", {"-f"}, true);
}

void openhd::reboot::systemctl_power(bool shutdownOnly) {
  openhd::log::get_default()->warn(
      "Power command received (shutdownOnly:{}), using aggressive sequence",
      shutdownOnly);
  OHDUtil::run_command("sync", {}, true);

  if (shutdownOnly) {
    // Some Images don't allow soft restarts or reboots when a netork is
    // connected
    if ((OHDPlatform::instance().platform_type ==
         X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) ||
        (OHDPlatform::instance().platform_type ==
         X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3)) {
      command_shutdown();
    } else {
      systemctl_shutdown();
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    systemctl_force_shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    command_shutdown();
  } else {
    if ((OHDPlatform::instance().platform_type ==
         X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W) ||
        (OHDPlatform::instance().platform_type ==
         X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3)) {
      command_reboot();
    } else {
      systemctl_reboot();
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    systemctl_force_reboot();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    command_reboot();
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
