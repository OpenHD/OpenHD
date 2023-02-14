//
// Created by consti10 on 14.02.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_

#include <chrono>

// convenient utils to handle the shutdown or reboot commands for OpenHD.
namespace openhd::reboot{

// NOTE: Might block calling thread
void systemctl_shutdown();
void systemctl_reboot();
// Reboot or shutdown
void systemctl_power(bool shutdownOnly);

// Returns immediately and performs the shutdown / reboot action after a given delay (can be 0)
void handle_power_command_async(std::chrono::milliseconds delay, bool shutdownOnly);
}

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_
