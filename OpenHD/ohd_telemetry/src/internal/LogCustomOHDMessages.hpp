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

// Every OHD mavlink telemetry message should have a verbose loggin method
//

#ifndef XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
#define XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H

#include <cassert>
#include <iostream>

#include "../mav_helper.h"
#include "../mav_include.h"
#include "openhd_spdlog.h"

/**
 * Helper for converting ohd custom messages to mavlink and printing them to
 * stdout
 */
namespace LogCustomOHDMessages {

static void logOnboardComputerStatus(
    const mavlink_onboard_computer_status_t &decoded) {
  std::stringstream ss;
  ss << "MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS: cpu_usage:"
     << (int)decoded.cpu_cores[0]
     << " temp:" << (int)decoded.temperature_core[0];
  openhd::log::debug_log(ss.str());
}

static void logOpenHDMessages(const std::vector<MavlinkMessage> &msges) {
  for (const auto &msg : msges) {
    if (msg.m.msgid == MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS) {
      mavlink_onboard_computer_status_t decoded;
      mavlink_msg_onboard_computer_status_decode(&msg.m, &decoded);
      logOnboardComputerStatus(decoded);
    } else {
      std::stringstream ss;
      ss << "unknown ohd msg with msgid:" << (int)msg.m.msgid;
      openhd::log::debug_log(ss.str());
    }
  }
}
}  // namespace LogCustomOHDMessages

#endif  // XMAVLINKSERVICE_LOGCUSTOMOHDMESSAGES_H
