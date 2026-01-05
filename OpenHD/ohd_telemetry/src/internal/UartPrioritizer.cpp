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

#include "UartPrioritizer.h"

#include <algorithm>

bool UartPrioritizer::valid_priority_value(int value) {
  return value >= 0 && value <= 10;
}

int UartPrioritizer::determine_priority(
    const MavlinkMessage& message, const UartPriorityProfile& profile) {
  const auto& msg = message.m;
  if (msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE ||
      msg.msgid == MAVLINK_MSG_ID_MANUAL_CONTROL ||
      msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
    return profile.rc_priority;
  }
  if (msg.sysid == OHD_SYS_ID_AIR || msg.sysid == OHD_SYS_ID_GROUND) {
    return profile.openhd_priority;
  }
  if (msg.sysid == OHD_SYS_ID_FC || msg.sysid == OHD_SYS_ID_FC_BETAFLIGHT) {
    return profile.flight_controller_priority;
  }
  return profile.default_priority;
}

std::vector<MavlinkMessage> UartPrioritizer::sort_by_priority(
    const std::vector<MavlinkMessage>& messages,
    const UartPriorityProfile& profile) const {
  std::vector<std::pair<MavlinkMessage, int>> prioritized;
  prioritized.reserve(messages.size());
  for (const auto& message : messages) {
    prioritized.emplace_back(message,
                             determine_priority(message, profile));
  }
  std::stable_sort(
      prioritized.begin(), prioritized.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });
  std::vector<MavlinkMessage> sorted;
  sorted.reserve(prioritized.size());
  for (auto& entry : prioritized) {
    sorted.push_back(entry.first);
  }
  return sorted;
}
