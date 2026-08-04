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

#include "openhd_sock.h"

#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "include_json.hpp"
#include "openhd_spdlog.h"

namespace {

constexpr const char* kSocketPath = "/run/openhd/openhd_sys.sock";
constexpr size_t kMaxLineLength = 4096;

std::shared_ptr<spdlog::logger> openhd_sock_logger() {
  static std::shared_ptr<spdlog::logger> logger =
      openhd::log::create_or_get("openhd_sock");
  return logger;
}

bool write_all(int fd, const void* data, size_t len) {
  const auto* ptr = static_cast<const char*>(data);
  size_t remaining = len;
  while (remaining > 0) {
    const ssize_t written = ::send(fd, ptr, remaining, MSG_NOSIGNAL);
    if (written < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    ptr += written;
    remaining -= static_cast<size_t>(written);
  }
  return true;
}

std::optional<std::string> read_line_with_timeout(
    int fd, std::chrono::milliseconds timeout) {
  std::string buffer;
  buffer.reserve(256);
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  char tmp[256];

  while (buffer.size() < kMaxLineLength) {
    auto now = std::chrono::steady_clock::now();
    if (now >= deadline) {
      return std::nullopt;
    }

    const auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLIN;
    const int ready = ::poll(&pfd, 1, static_cast<int>(remaining.count()));
    if (ready < 0) {
      if (errno == EINTR) {
        continue;
      }
      return std::nullopt;
    }
    if (ready == 0) {
      return std::nullopt;
    }

    const ssize_t count = ::recv(fd, tmp, sizeof(tmp), 0);
    if (count > 0) {
      buffer.append(tmp, static_cast<size_t>(count));
      const auto pos = buffer.find('\n');
      if (pos != std::string::npos) {
        return buffer.substr(0, pos);
      }
      continue;
    }
    if (count == 0) {
      return std::nullopt;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
      continue;
    }
    return std::nullopt;
  }

  return std::nullopt;
}

bool can_connect_sysutils() {
  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    return false;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  const bool ok =
      (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
  ::close(fd);
  return ok;
}

}  // namespace

namespace openhd {

Reporter& Reporter::instance() {
  static Reporter instance{};
  return instance;
}

Reporter::Reporter()
    : m_pending_send(false),
      m_shutdown(false),
      m_last_sent(std::chrono::steady_clock::time_point::min()),
      m_refresh_interval(std::chrono::hours(24)),
      m_status_message_refresh_interval(std::chrono::seconds(5)),
      m_worker(&Reporter::worker_loop, this) {}

Reporter::~Reporter() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shutdown = true;
    m_condition.notify_all();
  }
  if (m_worker.joinable()) {
    m_worker.join();
  }
}

void Reporter::report(State state, const std::string& description, int ttl_ms) {
  Status new_status{state, ttl_ms, description};
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_status.has_value() && m_status->state == new_status.state &&
        m_status->ttl_ms == new_status.ttl_ms &&
        m_status->description == new_status.description) {
      const auto now = std::chrono::steady_clock::now();
      if (now - m_last_sent < m_refresh_interval) {
        return;
      }
    }
    m_status = new_status;
    m_pending_send = true;
  }
  m_condition.notify_one();
  send_pending_now();
}

void Reporter::report_status(const std::string& code,
                             const std::string& description, int ttl_ms) {
  const auto now = std::chrono::steady_clock::now();
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_last_status_messages.find(code);
    if (it != m_last_status_messages.end() &&
        now - it->second < m_status_message_refresh_interval) {
      return;
    }
    m_last_status_messages[code] = now;
  }
  nlohmann::json payload;
  payload["type"] = "indicator.status";
  payload["source"] = "openhd";
  payload["code"] = code;
  payload["description"] = description;
  payload["ttl_ms"] = ttl_ms;
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
}

void Reporter::clear() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status.reset();
    m_pending_send = true;
  }
  m_condition.notify_one();
  send_pending_now();
}

void Reporter::worker_loop() {
  while (true) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_status.has_value() && !m_pending_send) {
      m_condition.wait(lock, [this]() {
        return m_shutdown || m_pending_send || m_status.has_value();
      });
    } else {
      auto next_refresh = m_last_sent + m_refresh_interval;
      m_condition.wait_until(lock, next_refresh,
                             [this]() { return m_shutdown || m_pending_send; });
    }
    if (m_shutdown) {
      return;
    }
    std::optional<Status> status_copy;
    const bool should_send = prepare_send_locked(status_copy);
    lock.unlock();

    if (!should_send) {
      continue;
    }
    if (status_copy.has_value()) {
      send_state(status_copy.value());
    } else {
      send_clear();
    }
  }
}

void Reporter::send_state(const Status& status) {
  const auto state_string = state_to_string(status.state);
  if (state_string == "UNKNOWN") {
    openhd_sock_logger()->warn(
        "Skipping indicator state update: unknown state value {}",
        static_cast<int>(status.state));
    return;
  }
  std::string description = state_string;
  if (!status.description.empty()) {
    description += " (" + status.description + ")";
  }
  nlohmann::json payload;
  payload["type"] = "indicator.set";
  payload["source"] = "openhd";
  payload["state"] = state_string;
  payload["description"] = description;
  payload["ttl_ms"] = status.ttl_ms;
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
  m_last_sent = std::chrono::steady_clock::now();
}

void Reporter::send_clear() {
  nlohmann::json payload;
  payload["type"] = "indicator.clear";
  payload["source"] = "openhd";
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
  m_last_sent = std::chrono::steady_clock::now();
}

bool Reporter::send_payload(const std::string& serialized_payload) {
  const auto path = socket_path();
  if (path.size() >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("indicator socket path too long: {}", path);
    return false;
  }
  std::error_code ec;
  const auto parent = std::filesystem::path(path).parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent, ec);
    if (ec) {
      openhd_sock_logger()->debug(
          "unable to create indicator socket dir {}: {}", parent.string(),
          ec.message());
    }
  }

  // The status reader listens with a blocking AF_UNIX/STREAM server. Use a
  // simple blocking connect/write loop for maximum compatibility.
  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("indicator socket creation failed: {}",
                                strerror(errno));
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("indicator socket connect failed: {}",
                                strerror(errno));
    close(fd);
    return false;
  }

  const bool sent_ok =
      write_all(fd, serialized_payload.data(), serialized_payload.size());
  close(fd);
  if (!sent_ok) {
    openhd_sock_logger()->debug("indicator send failed: {}", strerror(errno));
    return false;
  }
  return true;
}

void Reporter::send_pending_now() {
  std::optional<Status> status_copy;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    status_copy = m_status;
    m_pending_send = false;
  }
  if (status_copy.has_value()) {
    send_state(status_copy.value());
  } else {
    send_clear();
  }
}

bool Reporter::prepare_send_locked(std::optional<Status>& status_copy) {
  bool should_send = m_pending_send;
  m_pending_send = false;
  status_copy = m_status;
  return should_send;
}

std::string Reporter::state_to_string(State state) {
  switch (state) {
    case State::Booting:
      return "BOOTING";
    case State::Starting:
      return "STARTING";
    case State::Ready:
      return "READY";
    case State::LinkLost:
      return "LINK_LOST";
    case State::Error:
      return "ERROR";
    case State::Stopped:
      return "STOPPED";
  }
  return "UNKNOWN";
}

std::string Reporter::socket_path() { return kSocketPath; }

std::optional<int> request_platform_type(std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.platform.request";
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("platform socket path too long: {}",
                                kSocketPath);
    return std::nullopt;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("platform socket creation failed: {}",
                                strerror(errno));
    return std::nullopt;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("platform socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("platform request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("platform response timed out");
    return std::nullopt;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("platform response parse failed");
    return std::nullopt;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.platform.response") {
    openhd_sock_logger()->debug("unexpected platform response payload");
    return std::nullopt;
  }
  if (!parsed.contains("platform_type")) {
    openhd_sock_logger()->debug("platform response missing platform_type");
    return std::nullopt;
  }

  return parsed["platform_type"].get<int>();
}

std::optional<SysutilSettings> request_sysutil_settings(
    std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.settings.request";
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("settings socket path too long: {}",
                                kSocketPath);
    return std::nullopt;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("settings socket creation failed: {}",
                                strerror(errno));
    return std::nullopt;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("settings socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("settings request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("settings response timed out");
    return std::nullopt;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("settings response parse failed");
    return std::nullopt;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.settings.response") {
    openhd_sock_logger()->debug("unexpected settings response payload");
    return std::nullopt;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("settings response reported failure");
    return std::nullopt;
  }

  SysutilSettings settings{};
  auto read_string = [&](const char* key, const std::string& fallback) {
    auto it = parsed.find(key);
    if (it != parsed.end() && it->is_string()) {
      return it->get<std::string>();
    }
    return fallback;
  };
  auto read_bool = [&](const char* key, bool fallback) {
    auto it = parsed.find(key);
    if (it != parsed.end() && it->is_boolean()) {
      return it->get<bool>();
    }
    return fallback;
  };
  auto read_int = [&](const char* key, int fallback) {
    auto it = parsed.find(key);
    if (it != parsed.end() && it->is_number_integer()) {
      return it->get<int>();
    }
    return fallback;
  };
  if (parsed.contains("reset_requested")) {
    settings.reset_requested = parsed.value("reset_requested", false);
    settings.has_reset = parsed.value("has_reset", true);
  } else {
    settings.has_reset = parsed.value("has_reset", false);
  }

  if (parsed.contains("camera_type")) {
    settings.camera_type = parsed.value("camera_type", 0);
    settings.has_camera_type = parsed.value("has_camera_type", true);
  } else {
    settings.has_camera_type = parsed.value("has_camera_type", false);
  }
  if (parsed.contains("camera2_type")) {
    settings.camera2_type = parsed.value("camera2_type", 0);
    settings.has_camera2_type = parsed.value("has_camera2_type", true);
  } else {
    settings.has_camera2_type = parsed.value("has_camera2_type", false);
  }
  if (parsed.contains("camera_resolution_fps")) {
    settings.camera_resolution_fps =
        parsed.value("camera_resolution_fps", std::string{});
    settings.has_camera_resolution_fps =
        parsed.value("has_camera_resolution_fps", true);
  } else {
    settings.has_camera_resolution_fps =
        parsed.value("has_camera_resolution_fps", false);
  }
  if (parsed.contains("camera2_resolution_fps")) {
    settings.camera2_resolution_fps =
        parsed.value("camera2_resolution_fps", std::string{});
    settings.has_camera2_resolution_fps =
        parsed.value("has_camera2_resolution_fps", true);
  } else {
    settings.has_camera2_resolution_fps =
        parsed.value("has_camera2_resolution_fps", false);
  }
  settings.ip_camera_address =
      read_string("ip_camera_address", settings.ip_camera_address);
  settings.has_ip_camera_address = parsed.value(
      "has_ip_camera_address", parsed.contains("ip_camera_address"));
  settings.ip_camera_pipeline =
      read_string("ip_camera_pipeline", settings.ip_camera_pipeline);
  settings.has_ip_camera_pipeline = parsed.value(
      "has_ip_camera_pipeline", parsed.contains("ip_camera_pipeline"));
  settings.camera2_ip_camera_address = read_string(
      "camera2_ip_camera_address", settings.camera2_ip_camera_address);
  settings.has_camera2_ip_camera_address =
      parsed.value("has_camera2_ip_camera_address",
                   parsed.contains("camera2_ip_camera_address"));
  settings.camera2_ip_camera_pipeline = read_string(
      "camera2_ip_camera_pipeline", settings.camera2_ip_camera_pipeline);
  settings.has_camera2_ip_camera_pipeline =
      parsed.value("has_camera2_ip_camera_pipeline",
                   parsed.contains("camera2_ip_camera_pipeline"));
  settings.ip_camera_bitrate_mbits =
      read_int("ip_camera_bitrate_mbits", settings.ip_camera_bitrate_mbits);
  settings.has_ip_camera_bitrate_mbits =
      parsed.value("has_ip_camera_bitrate_mbits",
                   parsed.contains("ip_camera_bitrate_mbits"));

  const auto run_mode = parsed.value("run_mode", "");
  if (run_mode == "air" || run_mode == "ground" || run_mode == "record") {
    settings.run_as_air = (run_mode == "air" || run_mode == "record");
    settings.run_record_only = (run_mode == "record");
    settings.has_run_mode = parsed.value("has_run_mode", true);
  } else {
    settings.has_run_mode = parsed.value("has_run_mode", false);
  }

  settings.wifi_enable_autodetect =
      read_bool("wifi_enable_autodetect", settings.wifi_enable_autodetect);
  settings.wifi_wb_link_cards =
      read_string("wifi_wb_link_cards", settings.wifi_wb_link_cards);
  settings.wifi_hotspot_card =
      read_string("wifi_hotspot_card", settings.wifi_hotspot_card);
  settings.wifi_monitor_card_emulate = read_bool(
      "wifi_monitor_card_emulate", settings.wifi_monitor_card_emulate);
  settings.wifi_force_no_link_but_hotspot =
      read_bool("wifi_force_no_link_but_hotspot",
                settings.wifi_force_no_link_but_hotspot);
  settings.wifi_local_network_enable = read_bool(
      "wifi_local_network_enable", settings.wifi_local_network_enable);
  settings.wifi_local_network_ssid =
      read_string("wifi_local_network_ssid", settings.wifi_local_network_ssid);
  settings.wifi_local_network_password = read_string(
      "wifi_local_network_password", settings.wifi_local_network_password);
  settings.nw_ethernet_card =
      read_string("nw_ethernet_card", settings.nw_ethernet_card);
  settings.nw_manual_forwarding_ips = read_string(
      "nw_manual_forwarding_ips", settings.nw_manual_forwarding_ips);
  settings.nw_forward_to_localhost_58xx = read_bool(
      "nw_forward_to_localhost_58xx", settings.nw_forward_to_localhost_58xx);
  settings.ground_unit_ip =
      read_string("ground_unit_ip", settings.ground_unit_ip);
  settings.air_unit_ip = read_string("air_unit_ip", settings.air_unit_ip);
  settings.video_port = read_int("video_port", settings.video_port);
  settings.telemetry_port = read_int("telemetry_port", settings.telemetry_port);
  settings.disable_microhard_detection = read_bool(
      "disable_microhard_detection", settings.disable_microhard_detection);
  settings.force_microhard =
      read_bool("force_microhard", settings.force_microhard);
  settings.microhard_username =
      read_string("microhard_username", settings.microhard_username);
  settings.microhard_password =
      read_string("microhard_password", settings.microhard_password);
  settings.microhard_ip_air =
      read_string("microhard_ip_air", settings.microhard_ip_air);
  settings.microhard_ip_ground =
      read_string("microhard_ip_ground", settings.microhard_ip_ground);
  settings.microhard_ip_range =
      read_string("microhard_ip_range", settings.microhard_ip_range);
  settings.microhard_video_port =
      read_int("microhard_video_port", settings.microhard_video_port);
  settings.microhard_telemetry_port =
      read_int("microhard_telemetry_port", settings.microhard_telemetry_port);
  settings.gen_enable_last_known_position =
      read_bool("gen_enable_last_known_position",
                settings.gen_enable_last_known_position);
  settings.gen_rf_metrics_level =
      read_int("gen_rf_metrics_level", settings.gen_rf_metrics_level);

  return settings;
}

bool update_sysutil_settings(const SysutilSettingsUpdate& update,
                             std::chrono::milliseconds timeout) {
  if (!update.reset_requested.has_value() && !update.camera_type.has_value() &&
      !update.camera2_type.has_value() &&
      !update.camera_resolution_fps.has_value() &&
      !update.camera2_resolution_fps.has_value() &&
      !update.ip_camera_address.has_value() &&
      !update.ip_camera_pipeline.has_value() &&
      !update.camera2_ip_camera_address.has_value() &&
      !update.camera2_ip_camera_pipeline.has_value() &&
      !update.ip_camera_bitrate_mbits.has_value() &&
      !update.run_as_air.has_value() && !update.run_mode.has_value()) {
    return true;
  }

  nlohmann::json request;
  request["type"] = "sysutil.settings.update";
  if (update.reset_requested.has_value()) {
    request["reset_requested"] = update.reset_requested.value();
  }
  if (update.camera_type.has_value()) {
    request["camera_type"] = update.camera_type.value();
  }
  if (update.camera2_type.has_value()) {
    request["camera2_type"] = update.camera2_type.value();
  }
  if (update.camera_resolution_fps.has_value()) {
    request["camera_resolution_fps"] = update.camera_resolution_fps.value();
  }
  if (update.camera2_resolution_fps.has_value()) {
    request["camera2_resolution_fps"] = update.camera2_resolution_fps.value();
  }
  if (update.ip_camera_address.has_value()) {
    request["ip_camera_address"] = update.ip_camera_address.value();
  }
  if (update.ip_camera_pipeline.has_value()) {
    request["ip_camera_pipeline"] = update.ip_camera_pipeline.value();
  }
  if (update.camera2_ip_camera_address.has_value()) {
    request["camera2_ip_camera_address"] =
        update.camera2_ip_camera_address.value();
  }
  if (update.camera2_ip_camera_pipeline.has_value()) {
    request["camera2_ip_camera_pipeline"] =
        update.camera2_ip_camera_pipeline.value();
  }
  if (update.ip_camera_bitrate_mbits.has_value()) {
    request["ip_camera_bitrate_mbits"] = update.ip_camera_bitrate_mbits.value();
  }
  if (update.run_mode.has_value()) {
    request["run_mode"] = update.run_mode.value();
  } else if (update.run_as_air.has_value()) {
    request["run_mode"] = update.run_as_air.value() ? "air" : "ground";
  }
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("settings socket path too long: {}",
                                kSocketPath);
    return false;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("settings socket creation failed: {}",
                                strerror(errno));
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("settings socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("settings update send failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("settings update response timed out");
    return false;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("settings update response parse failed");
    return false;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.settings.update.response") {
    openhd_sock_logger()->debug("unexpected settings update response payload");
    return false;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("settings update response reported failure");
    return false;
  }

  return true;
}

bool request_sysutil_camera_setup(int camera_type,
                                  std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.camera.setup.request";
  request["camera_type"] = camera_type;
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("camera setup socket path too long: {}",
                                kSocketPath);
    return false;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("camera setup socket creation failed: {}",
                                strerror(errno));
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("camera setup socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("camera setup request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("camera setup response timed out");
    return false;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("camera setup response parse failed");
    return false;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.camera.setup.response") {
    openhd_sock_logger()->debug("unexpected camera setup response payload");
    return false;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("camera setup response reported failure");
    return false;
  }

  return true;
}

std::optional<std::vector<SysutilWifiCardInfo>> request_sysutil_wifi_cards(
    std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.wifi.request";
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("wifi socket path too long: {}", kSocketPath);
    return std::nullopt;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("wifi socket creation failed: {}",
                                strerror(errno));
    return std::nullopt;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("wifi socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("wifi request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("wifi response timed out");
    return std::nullopt;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("wifi response parse failed");
    return std::nullopt;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.wifi.response") {
    openhd_sock_logger()->debug("unexpected wifi response payload");
    return std::nullopt;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("wifi response reported failure");
    return std::nullopt;
  }

  std::vector<SysutilWifiCardInfo> cards;
  if (!parsed.contains("cards") || !parsed["cards"].is_array()) {
    return cards;
  }
  for (const auto& entry : parsed["cards"]) {
    SysutilWifiCardInfo card{};
    card.interface_name = entry.value("interface", "");
    card.driver_name = entry.value("driver", "");
    card.mac = entry.value("mac", "");
    card.phy_index = entry.value("phy_index", -1);
    card.vendor_id = entry.value("vendor_id", "");
    card.device_id = entry.value("device_id", "");
    card.type = entry.value("type", "");
    card.disabled = entry.value("disabled", false);
    card.card_name = entry.value("card_name", "");
    card.power_mode = entry.value("power_mode", "");
    card.power_level = entry.value("power_level", "");
    card.power_lowest = entry.value("power_lowest", "");
    card.power_low = entry.value("power_low", "");
    card.power_mid = entry.value("power_mid", "");
    card.power_high = entry.value("power_high", "");
    card.power_min = entry.value("power_min", "");
    card.power_max = entry.value("power_max", "");
    cards.push_back(std::move(card));
  }
  return cards;
}

bool request_sysutil_wifi_refresh(std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.wifi.update";
  request["action"] = "detect";
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("wifi update socket path too long: {}",
                                kSocketPath);
    return false;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("wifi update socket creation failed: {}",
                                strerror(errno));
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("wifi update socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("wifi update request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("wifi update response timed out");
    return false;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("wifi update response parse failed");
    return false;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.wifi.update.response") {
    openhd_sock_logger()->debug("unexpected wifi update response payload");
    return false;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("wifi update response reported failure");
    return false;
  }
  return true;
}

bool request_sysutil_artosyn_restart(std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.wifi.update";
  request["action"] = "restart_artosyn";
  auto serialized = request.dump();
  serialized.push_back('\n');

  if (strlen(kSocketPath) >= sizeof(sockaddr_un::sun_path)) {
    openhd_sock_logger()->debug("artosyn restart socket path too long: {}",
                                kSocketPath);
    return false;
  }

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    openhd_sock_logger()->debug("artosyn restart socket creation failed: {}",
                                strerror(errno));
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("artosyn restart socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  const bool sent_ok = write_all(fd, serialized.data(), serialized.size());
  if (!sent_ok) {
    openhd_sock_logger()->debug("artosyn restart request send failed: {}",
                                strerror(errno));
    ::close(fd);
    return false;
  }

  auto line_opt = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line_opt.has_value()) {
    openhd_sock_logger()->debug("artosyn restart response timed out");
    return false;
  }

  auto parsed = nlohmann::json::parse(*line_opt, nullptr, false);
  if (parsed.is_discarded()) {
    openhd_sock_logger()->debug("artosyn restart response parse failed");
    return false;
  }
  if (!parsed.contains("type") ||
      parsed.value("type", "") != "sysutil.wifi.update.response") {
    openhd_sock_logger()->debug("unexpected artosyn restart response payload");
    return false;
  }
  if (parsed.contains("ok") && !parsed.value("ok", true)) {
    openhd_sock_logger()->debug("artosyn restart response reported failure");
    return false;
  }
  return true;
}

bool wait_for_sysutils(std::chrono::milliseconds timeout,
                       std::chrono::milliseconds poll_interval) {
  if (timeout <= std::chrono::milliseconds::zero()) {
    return false;
  }

  const auto deadline = std::chrono::steady_clock::now() + timeout;
  bool logged = false;
  while (true) {
    std::error_code ec;
    if (std::filesystem::exists(kSocketPath, ec) && !ec) {
      if (can_connect_sysutils()) {
        return true;
      }
    }

    const auto now = std::chrono::steady_clock::now();
    if (now >= deadline) {
      return false;
    }

    if (!logged) {
      openhd_sock_logger()->info(
          "Waiting for sysutils socket ({} ms timeout)...", timeout.count());
      logged = true;
    }

    const auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    const auto sleep_for = std::min(
        poll_interval, std::max(remaining, std::chrono::milliseconds(1)));
    std::this_thread::sleep_for(sleep_for);
  }
}

SysutilStorageFormatResult request_sysutil_storage_format(
    uint8_t storage_id, std::chrono::milliseconds timeout) {
  SysutilStorageFormatResult result{};
  nlohmann::json request;
  request["type"] = "sysutil.storage.format.request";
  request["storage_id"] = storage_id;
  request["confirm"] = true;
  auto serialized = request.dump();
  serialized.push_back('\n');

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    result.message = "Cannot create sysutils socket.";
    return result;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);
  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    result.message = "Cannot connect to sysutils.";
    ::close(fd);
    return result;
  }
  if (!write_all(fd, serialized.data(), serialized.size())) {
    result.message = "Cannot send format request to sysutils.";
    ::close(fd);
    return result;
  }

  const auto line = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line) {
    result.message = "Sysutils format request timed out.";
    return result;
  }
  const auto parsed = nlohmann::json::parse(*line, nullptr, false);
  if (parsed.is_discarded() ||
      parsed.value("type", "") != "sysutil.storage.format.response") {
    result.message = "Invalid sysutils format response.";
    return result;
  }
  result.ok = parsed.value("ok", false);
  result.message = parsed.value(
      "message", result.ok ? "SD card format complete." : "SD card format failed.");
  return result;
}

std::optional<std::vector<SysutilStorageEntry>> request_sysutil_storage_list(
    std::chrono::milliseconds timeout) {
  nlohmann::json request;
  request["type"] = "sysutil.storage.list.request";
  auto serialized = request.dump();
  serialized.push_back('\n');

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    return std::nullopt;
  }
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);
  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0 ||
      !write_all(fd, serialized.data(), serialized.size())) {
    ::close(fd);
    return std::nullopt;
  }
  const auto line = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line) {
    return std::nullopt;
  }
  const auto parsed = nlohmann::json::parse(*line, nullptr, false);
  if (parsed.is_discarded() ||
      parsed.value("type", "") != "sysutil.storage.list.response" ||
      !parsed.contains("entries") || !parsed["entries"].is_array()) {
    return std::nullopt;
  }

  std::vector<SysutilStorageEntry> entries;
  for (const auto& item : parsed["entries"]) {
    if (!item.is_object()) {
      continue;
    }
    SysutilStorageEntry entry;
    entry.id = static_cast<uint8_t>(item.value("id", 0));
    entry.device = item.value("device", "");
    entry.kind = item.value("kind", "");
    entry.filesystem = item.value("filesystem", "");
    entry.label = item.value("label", "");
    entry.mountpoint = item.value("mountpoint", "");
    entry.size_bytes = item.value("size_bytes", uint64_t{0});
    entry.free_bytes = item.value("free_bytes", uint64_t{0});
    entry.mounted_at_video = item.value("mounted_at_video", false);
    entry.can_format = item.value("can_format", false);
    entry.can_repartition = item.value("can_repartition", false);
    entry.can_mount = item.value("can_mount", false);
    if (entry.id != 0 && !entry.device.empty()) {
      entries.push_back(std::move(entry));
    }
  }
  return entries;
}

SysutilStorageFormatResult request_sysutil_storage_action(
    uint8_t storage_id, const std::string& action,
    std::chrono::milliseconds timeout) {
  SysutilStorageFormatResult result{};
  nlohmann::json request;
  request["type"] = "sysutil.storage.action.request";
  request["storage_id"] = storage_id;
  request["action"] = action;
  request["confirm"] = true;
  auto serialized = request.dump();
  serialized.push_back('\n');

  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    result.message = "Cannot create sysutils socket.";
    return result;
  }
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);
  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0 ||
      !write_all(fd, serialized.data(), serialized.size())) {
    result.message = "Cannot send storage action to sysutils.";
    ::close(fd);
    return result;
  }
  const auto line = read_line_with_timeout(fd, timeout);
  ::close(fd);
  if (!line) {
    result.message = "Sysutils storage action timed out.";
    return result;
  }
  const auto parsed = nlohmann::json::parse(*line, nullptr, false);
  if (parsed.is_discarded() ||
      parsed.value("type", "") != "sysutil.storage.action.response") {
    result.message = "Invalid sysutils storage response.";
    return result;
  }
  result.ok = parsed.value("ok", false);
  result.message = parsed.value("message", "Storage action failed.");
  return result;
}

}  // namespace openhd
