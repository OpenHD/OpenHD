#pragma once

#include <array>
#include <cstdint>
#include <istream>
#include <optional>
#include <string>

namespace openhd::onboard {
struct CpuTimes {
  uint64_t total;
  uint64_t idle;
};

inline std::optional<CpuTimes> parse_cpu_times(std::istream& input) {
  std::string name;
  std::array<uint64_t, 8> ticks{};
  if (!(input >> name) || name != "cpu") return {};
  for (auto& value : ticks) {
    if (!(input >> value)) return {};
  }
  CpuTimes result{0, ticks[3] + ticks[4]};
  // guest/guest_nice are already included in user/nice. Count each tick once.
  for (auto value : ticks) result.total += value;
  return result;
}

inline std::optional<int> cpu_usage_between(CpuTimes before, CpuTimes after) {
  if (after.total <= before.total || after.idle < before.idle) return {};
  const auto total = after.total - before.total;
  const auto idle = after.idle - before.idle;
  if (idle > total) return {};
  return static_cast<int>(100.0 * (total - idle) / total + 0.5);
}
}  // namespace openhd::onboard
