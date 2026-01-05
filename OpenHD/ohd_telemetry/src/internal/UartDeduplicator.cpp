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

#include "UartDeduplicator.h"

UartDeduplicator::UartDeduplicator(std::chrono::milliseconds window)
    : m_window(window) {}

uint64_t UartDeduplicator::create_key(const mavlink_message_t& msg) const {
  // sysid, compid and msgid are enough to uniquely identify the source. Adding
  // the sequence number allows distinguishing between subsequent messages of
  // the same type.
  return (static_cast<uint64_t>(msg.sysid) << 40u) |
         (static_cast<uint64_t>(msg.compid) << 32u) |
         (static_cast<uint64_t>(msg.msgid) << 16u) |
         static_cast<uint64_t>(msg.seq);
}

void UartDeduplicator::prune_locked(
    std::chrono::steady_clock::time_point now) {
  for (auto it = m_recent_messages.begin(); it != m_recent_messages.end();) {
    if ((now - it->second) > m_window) {
      it = m_recent_messages.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<MavlinkMessage> UartDeduplicator::filter_and_mark(
    const std::vector<MavlinkMessage>& messages) {
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard<std::mutex> guard(m_mutex);
  prune_locked(now);
  std::vector<MavlinkMessage> filtered;
  filtered.reserve(messages.size());
  for (const auto& message : messages) {
    const auto key = create_key(message.m);
    const auto it = m_recent_messages.find(key);
    if (it != m_recent_messages.end() &&
        (now - it->second) <= m_window) {
      continue;
    }
    filtered.push_back(message);
    m_recent_messages[key] = now;
  }
  return filtered;
}
