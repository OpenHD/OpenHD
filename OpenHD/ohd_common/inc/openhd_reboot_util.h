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

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_

#include <chrono>

// convenient utils to handle the shutdown or reboot commands for OpenHD.
namespace openhd::reboot {

// NOTE: Might block calling thread
void systemctl_shutdown();
void systemctl_reboot();
// Reboot or shutdown
void systemctl_power(bool shutdownOnly);

// Returns immediately and performs the shutdown / reboot action after a given
// delay (can be 0) This delay usually exists to give a mavlink acknowledging
// response time to be sent to the ground
void handle_power_command_async(std::chrono::milliseconds delay,
                                bool shutdownOnly);

// TODO
// dirty
// OpenHD is written under the assumption that once discovered hardware works
// and does not disconnect / their linux driver(s) doesn't crash. Most common
// camera driver error(s) are handled gracefully, e.g. restarting the pipeline
// seems to fix most issues with the rpi camera drivers. For the wifi cards, it
// is a bit more complicated though. Here are the 2 most common issues - they
// are always fixable by using proper hardware, but we have the following checks
// in place anyways to account for the common mistake of improper wiring. 1)
// completely disconnecting and then reconnecting the wifi card
// -> can be fixed semi-gracefully by just crashing openhd, then letting the
// service re-start openhd 2) crashed / messed up wifi (driver ?!)
// -> requires a complete re-boot of linux
}  // namespace openhd::reboot

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_REBOOT_UTIL_H_
