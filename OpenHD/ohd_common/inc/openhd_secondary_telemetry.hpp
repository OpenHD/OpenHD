#ifndef OPENHD_SECONDARY_TELEMETRY_HPP_
#define OPENHD_SECONDARY_TELEMETRY_HPP_

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace openhd {

// Process-local health registry for telemetry transports which remain usable
// while the wifibroadcast radios are changing channel. A configured transport
// is not considered live until traffic has actually been received from its
// peer recently.
class SecondaryTelemetryStatus {
 public:
  static SecondaryTelemetryStatus& instance() {
    static SecondaryTelemetryStatus status;
    return status;
  }

  void set_configured(const std::string& name, bool configured) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& path = m_paths[name];
    path.configured = configured;
    if (!configured) path.last_rx = std::chrono::steady_clock::time_point{};
  }

  void note_received(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& path = m_paths[name];
    path.configured = true;
    path.last_rx = std::chrono::steady_clock::now();
  }

  [[nodiscard]] bool is_live(
      const std::string& name,
      std::chrono::milliseconds maximum_age = std::chrono::seconds(5)) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto found = m_paths.find(name);
    return found != m_paths.end() && found->second.configured &&
           found->second.last_rx != std::chrono::steady_clock::time_point{} &&
           std::chrono::steady_clock::now() - found->second.last_rx <=
               maximum_age;
  }

  [[nodiscard]] bool any_live(
      std::chrono::milliseconds maximum_age = std::chrono::seconds(5)) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto now = std::chrono::steady_clock::now();
    for (const auto& [name, path] : m_paths) {
      (void)name;
      if (path.configured &&
          path.last_rx != std::chrono::steady_clock::time_point{} &&
          now - path.last_rx <= maximum_age) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] std::vector<std::string> live_paths(
      std::chrono::milliseconds maximum_age = std::chrono::seconds(5)) const {
    std::vector<std::string> result;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto now = std::chrono::steady_clock::now();
    for (const auto& [name, path] : m_paths) {
      if (path.configured &&
          path.last_rx != std::chrono::steady_clock::time_point{} &&
          now - path.last_rx <= maximum_age) {
        result.push_back(name);
      }
    }
    return result;
  }

 private:
  struct PathState {
    bool configured = false;
    std::chrono::steady_clock::time_point last_rx{};
  };

  mutable std::mutex m_mutex;
  std::unordered_map<std::string, PathState> m_paths;
};

}  // namespace openhd

#endif  // OPENHD_SECONDARY_TELEMETRY_HPP_
