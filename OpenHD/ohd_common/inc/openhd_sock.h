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
#include <unordered_map>

namespace openhd {

enum class State {
  Booting,
  Starting,
  Ready,
  LinkLost,
  Error,
  Stopped
};

class Reporter {
 public:
  static Reporter& instance();

  void report(State state, const std::string& description = "",
              int ttl_ms = 3000);
  void report_status(const std::string& code, const std::string& description,
                     int ttl_ms = 3000);
  void clear();

 private:
  Reporter();
  ~Reporter();
  Reporter(const Reporter&) = delete;
  Reporter& operator=(const Reporter&) = delete;

  struct Status {
    State state;
    int ttl_ms;
    std::string description;
  };

  void worker_loop();
  void send_state(const Status& status);
  void send_clear();
  bool send_payload(const std::string& serialized_payload);
  void send_pending_now();
  bool prepare_send_locked(std::optional<Status>& status_copy);
  static std::string state_to_string(State state);
  static std::string socket_path();

  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::optional<Status> m_status;
  bool m_pending_send;
  bool m_shutdown;
  std::chrono::steady_clock::time_point m_last_sent;
  const std::chrono::milliseconds m_refresh_interval;
  const std::chrono::milliseconds m_status_message_refresh_interval;
  std::thread m_worker;
  std::unordered_map<std::string, std::chrono::steady_clock::time_point>
      m_last_status_messages;
};

std::optional<int> request_platform_type(
    std::chrono::milliseconds timeout = std::chrono::seconds(1));

struct SysutilSettings {
  bool has_reset = false;
  bool reset_requested = false;
  bool has_camera_type = false;
  int camera_type = 0;
  bool has_run_mode = false;
  bool run_as_air = false;
  bool run_record_only = false;
};

struct SysutilSettingsUpdate {
  std::optional<bool> reset_requested;
  std::optional<int> camera_type;
  std::optional<bool> run_as_air;
  std::optional<std::string> run_mode;
};

std::optional<SysutilSettings> request_sysutil_settings(
    std::chrono::milliseconds timeout = std::chrono::seconds(1));
bool update_sysutil_settings(
    const SysutilSettingsUpdate& update,
    std::chrono::milliseconds timeout = std::chrono::seconds(1));

}  // namespace openhd

#endif  // OPENHD_SOCK_H
