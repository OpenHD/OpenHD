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
#include <vector>

namespace openhd {

enum class State { Booting, Starting, Ready, LinkLost, Error, Stopped };

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
  bool wifi_enable_autodetect = true;
  std::string wifi_wb_link_cards;
  std::string wifi_hotspot_card;
  bool wifi_monitor_card_emulate = false;
  bool wifi_force_no_link_but_hotspot = false;
  bool wifi_local_network_enable = false;
  std::string wifi_local_network_ssid;
  std::string wifi_local_network_password;
  std::string nw_ethernet_card = "RPI_ETHERNET_ONLY";
  std::string nw_manual_forwarding_ips;
  bool nw_forward_to_localhost_58xx = false;
  std::string ground_unit_ip;
  std::string air_unit_ip;
  int video_port = 5000;
  int telemetry_port = 5600;
  bool disable_microhard_detection = false;
  bool force_microhard = false;
  std::string microhard_username = "admin";
  std::string microhard_password = "qwertz1";
  std::string microhard_ip_air;
  std::string microhard_ip_ground;
  std::string microhard_ip_range;
  int microhard_video_port = 5910;
  int microhard_telemetry_port = 5920;
  bool gen_enable_last_known_position = false;
  int gen_rf_metrics_level = 0;
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
bool request_sysutil_camera_setup(
    int camera_type,
    std::chrono::milliseconds timeout = std::chrono::seconds(2));

bool wait_for_sysutils(
    std::chrono::milliseconds timeout = std::chrono::seconds(30),
    std::chrono::milliseconds poll_interval = std::chrono::milliseconds(200));

struct SysutilWifiCardInfo {
  std::string interface_name;
  std::string driver_name;
  std::string mac;
  int phy_index = -1;
  std::string vendor_id;
  std::string device_id;
  std::string type;
  bool disabled = false;
  std::string card_name;
  std::string power_mode;
  std::string power_level;
  std::string power_lowest;
  std::string power_low;
  std::string power_mid;
  std::string power_high;
  std::string power_min;
  std::string power_max;
};

std::optional<std::vector<SysutilWifiCardInfo>> request_sysutil_wifi_cards(
    std::chrono::milliseconds timeout = std::chrono::seconds(1));
bool request_sysutil_wifi_refresh(
    std::chrono::milliseconds timeout = std::chrono::seconds(2));

}  // namespace openhd

#endif  // OPENHD_SOCK_H
