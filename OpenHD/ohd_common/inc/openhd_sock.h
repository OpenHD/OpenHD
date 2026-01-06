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

#ifndef OPENHD_SOCK_H
#define OPENHD_SOCK_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace openhd {

enum class IndicatorState {
  Booting,
  Starting,
  Ready,
  LinkLost,
  Error,
  Stopped
};

class IndicatorReporter {
 public:
  static IndicatorReporter& instance();

  void report_state(IndicatorState state, int severity = 0,
                    int ttl_ms = 3000);
  void clear();

 private:
  IndicatorReporter();
  ~IndicatorReporter();
  IndicatorReporter(const IndicatorReporter&) = delete;
  IndicatorReporter& operator=(const IndicatorReporter&) = delete;

  struct IndicatorStatus {
    IndicatorState state;
    int severity;
    int ttl_ms;
  };

  void worker_loop();
  void send_state(const IndicatorStatus& status);
  void send_clear();
  bool send_payload(const std::string& serialized_payload);
  static std::string state_to_string(IndicatorState state);
  static std::string socket_path();

  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::optional<IndicatorStatus> m_status;
  bool m_pending_send;
  bool m_shutdown;
  std::chrono::steady_clock::time_point m_last_sent;
  const std::chrono::milliseconds m_refresh_interval;
  std::thread m_worker;
};

}  // namespace openhd

#endif  // OPENHD_SOCK_H
