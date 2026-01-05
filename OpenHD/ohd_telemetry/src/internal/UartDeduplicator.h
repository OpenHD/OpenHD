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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTDEDUPLICATOR_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTDEDUPLICATOR_H_

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "mav_include.h"

/**
 * Keeps track of recently forwarded mavlink messages and drops duplicates
 * regardless of which link they arrive on (wifibroadcast or UART). This lets
 * us transmit on multiple paths while only forwarding a single copy to
 * consumers.
 */
class UartDeduplicator {
 public:
  explicit UartDeduplicator(
      std::chrono::milliseconds window = std::chrono::milliseconds(1500));

  std::vector<MavlinkMessage> filter_and_mark(
      const std::vector<MavlinkMessage>& messages);

 private:
  [[nodiscard]] uint64_t create_key(const mavlink_message_t& msg) const;
  void prune_locked(std::chrono::steady_clock::time_point now);

 private:
  std::unordered_map<uint64_t, std::chrono::steady_clock::time_point>
      m_recent_messages;
  std::chrono::milliseconds m_window;
  std::mutex m_mutex;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_UARTDEDUPLICATOR_H_
