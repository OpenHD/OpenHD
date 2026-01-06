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

#include "openhd_indicator_reporter.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <memory>
#include <string>

#include "include_json.hpp"
#include "openhd_spdlog.h"

namespace {

std::shared_ptr<spdlog::logger> indicator_logger() {
  static std::shared_ptr<spdlog::logger> logger =
      openhd::log::create_or_get("indicator");
  return logger;
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
      m_refresh_interval(std::chrono::milliseconds(2000)),
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
}

void IndicatorReporter::clear() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status.reset();
    m_pending_send = true;
  }
  m_condition.notify_one();
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
    const auto now = std::chrono::steady_clock::now();
    const bool should_refresh = m_status.has_value() &&
                                now - m_last_sent >= m_refresh_interval;
    bool should_send = m_pending_send || should_refresh;
    m_pending_send = false;
    auto status_copy = m_status;
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
    indicator_logger()->debug("indicator socket path too long: {}", path);
    return false;
  }
  const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    indicator_logger()->debug("indicator socket creation failed: {}", strerror(errno));
    return false;
  }

  int flags = fcntl(fd, F_GETFL, 0);
  if (flags != -1) {
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }

  sockaddr_un addr {};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  int result = ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  if (result < 0 && errno == EINPROGRESS) {
    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLOUT;
    result = ::poll(&pfd, 1, 100);
    if (result > 0) {
      int socket_error = 0;
      socklen_t len = sizeof(socket_error);
      if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_error, &len) < 0 ||
          socket_error != 0) {
        close(fd);
        return false;
      }
    } else {
      close(fd);
      return false;
    }
  } else if (result < 0) {
    close(fd);
    return false;
  }

  const ssize_t bytes_sent =
      ::send(fd, serialized_payload.data(), serialized_payload.size(),
             MSG_NOSIGNAL);
  close(fd);
  if (bytes_sent != static_cast<ssize_t>(serialized_payload.size())) {
    indicator_logger()->debug("indicator send failed, sent {} of {}", bytes_sent,
                              serialized_payload.size());
    return false;
  }
  return true;
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

std::string IndicatorReporter::socket_path() {
  return "/run/openhd/openhd_sys.sock";
}

}  // namespace openhd
