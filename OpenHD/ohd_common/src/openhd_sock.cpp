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

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <poll.h>
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
      m_condition.wait_until(lock, next_refresh, [this]() {
        return m_shutdown || m_pending_send;
      });
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

  sockaddr_un addr {};
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

  sockaddr_un addr {};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    openhd_sock_logger()->debug("platform socket connect failed: {}",
                                strerror(errno));
    ::close(fd);
    return std::nullopt;
  }

  const bool sent_ok =
      write_all(fd, serialized.data(), serialized.size());
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

  sockaddr_un addr {};
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

  const auto run_mode = parsed.value("run_mode", "");
  if (run_mode == "air" || run_mode == "ground" || run_mode == "record") {
    settings.run_as_air = (run_mode == "air" || run_mode == "record");
    settings.run_record_only = (run_mode == "record");
    settings.has_run_mode = parsed.value("has_run_mode", true);
  } else {
    settings.has_run_mode = parsed.value("has_run_mode", false);
  }

  return settings;
}

bool update_sysutil_settings(const SysutilSettingsUpdate& update,
                             std::chrono::milliseconds timeout) {
  if (!update.reset_requested.has_value() &&
      !update.camera_type.has_value() &&
      !update.run_as_air.has_value() &&
      !update.run_mode.has_value()) {
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

  sockaddr_un addr {};
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

}  // namespace openhd
