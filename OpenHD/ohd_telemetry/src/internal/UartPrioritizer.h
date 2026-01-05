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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTPRIORITIZER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTPRIORITIZER_H_

#include <vector>

#include "mav_include.h"

struct UartPriorityProfile {
  int rc_priority = 3;
  int openhd_priority = 2;
  int flight_controller_priority = 1;
  int default_priority = 0;
};

class UartPrioritizer {
 public:
  static bool valid_priority_value(int value);
  std::vector<MavlinkMessage> sort_by_priority(
      const std::vector<MavlinkMessage>& messages,
      const UartPriorityProfile& profile) const;

 private:
  static int determine_priority(const MavlinkMessage& message,
                                const UartPriorityProfile& profile);
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTPRIORITIZER_H_
