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
#include <string>

#include "include_json.hpp"
#include "openhd_spdlog.h"

namespace {

constexpr const char* kSocketPath = "/run/openhd/openhd_sys.sock";

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

}  // namespace

namespace openhd {

IndicatorReporter& IndicatorReporter::instance() {
  static IndicatorReporter instance{};
  return instance;
}

IndicatorReporter::IndicatorReporter()
    : m_pending_send(false),
      m_shutdown(false),
      m_last_sent(std::chrono::steady_clock::time_point::min()),
      m_refresh_interval(std::chrono::hours(24)),
      m_status_message_refresh_interval(std::chrono::seconds(5)),
      m_worker(&IndicatorReporter::worker_loop, this) {}

IndicatorReporter::~IndicatorReporter() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shutdown = true;
    m_condition.notify_all();
  }
  if (m_worker.joinable()) {
    m_worker.join();
  }
}

void IndicatorReporter::report_state(IndicatorState state, int severity,
                                     int ttl_ms) {
  IndicatorStatus new_status{state, severity, ttl_ms};
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_status.has_value() && m_status->state == new_status.state &&
        m_status->severity == new_status.severity &&
        m_status->ttl_ms == new_status.ttl_ms) {
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

void IndicatorReporter::report_status_message(const std::string& code,
                                              const std::string& message,
                                              int severity, int ttl_ms) {
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
  payload["message"] = message;
  payload["severity"] = severity;
  payload["ttl_ms"] = ttl_ms;
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
}

void IndicatorReporter::clear() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status.reset();
    m_pending_send = true;
  }
  m_condition.notify_one();
  send_pending_now();
}

void IndicatorReporter::worker_loop() {
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
    std::optional<IndicatorStatus> status_copy;
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

void IndicatorReporter::send_state(const IndicatorStatus& status) {
  nlohmann::json payload;
  payload["type"] = "indicator.set";
  payload["source"] = "openhd";
  payload["state"] = state_to_string(status.state);
  payload["severity"] = status.severity;
  payload["ttl_ms"] = status.ttl_ms;
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
  m_last_sent = std::chrono::steady_clock::now();
}

void IndicatorReporter::send_clear() {
  nlohmann::json payload;
  payload["type"] = "indicator.clear";
  payload["source"] = "openhd";
  auto serialized = payload.dump();
  serialized.push_back('\n');
  send_payload(serialized);
  m_last_sent = std::chrono::steady_clock::now();
}

bool IndicatorReporter::send_payload(const std::string& serialized_payload) {
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

void IndicatorReporter::send_pending_now() {
  std::optional<IndicatorStatus> status_copy;
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

bool IndicatorReporter::prepare_send_locked(
    std::optional<IndicatorStatus>& status_copy) {
  bool should_send = m_pending_send;
  m_pending_send = false;
  status_copy = m_status;
  return should_send;
}

std::string IndicatorReporter::state_to_string(IndicatorState state) {
  switch (state) {
    case IndicatorState::Booting:
      return "BOOTING";
    case IndicatorState::Starting:
      return "STARTING";
    case IndicatorState::Ready:
      return "READY";
    case IndicatorState::LinkLost:
      return "LINK_LOST";
    case IndicatorState::Error:
      return "ERROR";
    case IndicatorState::Stopped:
      return "STOPPED";
  }
  return "UNKNOWN";
}

std::string IndicatorReporter::socket_path() { return kSocketPath; }

}  // namespace openhd
