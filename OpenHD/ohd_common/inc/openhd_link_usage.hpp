#ifndef OPENHD_LINK_USAGE_HPP
#define OPENHD_LINK_USAGE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace openhd::link_usage {

// DATA96 type 0x4f, payload "OHLU", version 1. All rates and capacities are
// unsigned bits/second. The 16-byte header is followed by (category, rate)
// tuples, allowing future traffic categories without changing the MAVLink dialect.
constexpr uint8_t kMavlinkDataType = 0x4f;
constexpr uint8_t kPayloadVersion = 1;

enum class Category : uint8_t {
  Camera1 = 1, Camera2 = 2, Audio = 3, Telemetry = 4,
  Other = 5, Control = 6
};

struct Sample {
  std::string link_name;
  uint32_t capacity_bps = 0;  // 0 means unknown, never an invented capacity.
  uint32_t total_bps = 0;
  std::array<uint32_t, 7> category_bps{};
};

class Registry {
 public:
  static Registry& instance() {
    static Registry registry;
    return registry;
  }

  void add_link(const std::string& name, uint32_t capacity_bps = 0) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_links[name].capacity_bps = capacity_bps;
  }
  void remove_link(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_links.erase(name);
  }
  void set_capacity(const std::string& name, uint32_t capacity_bps) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_links.find(name);
    if (it != m_links.end()) it->second.capacity_bps = capacity_bps;
  }
  void record(const std::string& name, Category category, uint64_t bytes) {
    const auto index = static_cast<std::size_t>(category);
    if (index == 0 || index >= 7 || bytes == 0) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_links.find(name);
    if (it != m_links.end()) {
      it->second.bytes[index] += bytes;
      it->second.last_activity = std::chrono::steady_clock::now();
    }
  }

  std::vector<Sample> sample() {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto now = std::chrono::steady_clock::now();
    std::vector<Sample> result;
    result.reserve(m_links.size());
    for (auto& [name, counter] : m_links) {
      // A configured secondary transport is not necessarily connected. Only
      // advertise it after recent outbound traffic; the WiFi primary remains
      // visible even while idle.
      if (name != "WIFIBROADCAST" &&
          (counter.last_activity == std::chrono::steady_clock::time_point{} ||
           now - counter.last_activity > std::chrono::seconds(3))) {
        counter.last_sample = now;
        counter.previous_bytes = counter.bytes;
        continue;
      }
      const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - counter.last_sample).count();
      if (elapsed_ms < 100) continue;
      Sample sample{};
      sample.link_name = name;
      sample.capacity_bps = counter.capacity_bps;
      uint64_t total = 0;
      for (std::size_t i = 1; i < sample.category_bps.size(); ++i) {
        const auto delta = counter.bytes[i] - counter.previous_bytes[i];
        const auto rate = delta * 8000 / static_cast<uint64_t>(elapsed_ms);
        sample.category_bps[i] = static_cast<uint32_t>(
            std::min<uint64_t>(rate, UINT32_MAX));
        total += sample.category_bps[i];
        counter.previous_bytes[i] = counter.bytes[i];
      }
      sample.total_bps = static_cast<uint32_t>(
          std::min<uint64_t>(total, UINT32_MAX));
      counter.last_sample = now;
      result.push_back(sample);
    }
    return result;
  }

 private:
  struct Counter {
    uint32_t capacity_bps = 0;
    std::array<uint64_t, 7> bytes{};
    std::array<uint64_t, 7> previous_bytes{};
    std::chrono::steady_clock::time_point last_sample =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point last_activity{};
  };
  std::mutex m_mutex;
  std::map<std::string, Counter> m_links;
};

}  // namespace openhd::link_usage
#endif
