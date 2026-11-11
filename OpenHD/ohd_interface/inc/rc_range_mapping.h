#pragma once

#include <charconv>
#include <optional>
#include <string_view>
#include <vector>

namespace openhd::rc {

inline constexpr auto default_bw_ranges = "1500:20,2100:40";
inline constexpr auto default_mcs_ranges = "1300:0,1700:2,2100:3";
inline constexpr auto legacy_bw_ranges = "1300:10,1700:20,2100:40";
inline constexpr auto legacy_mcs_ranges = "1200:0,1400:1,1600:2,1800:3,2100:4";

enum class RangeKind { Bandwidth, Mcs };
struct Range {
  int upper;
  int value;
};

// Each upper bound is inclusive; the next area starts at upper + 1.
// A single PARAM_EXT string replaces the entire map atomically.
inline std::optional<std::vector<Range>> parse_ranges(std::string_view text,
                                                      RangeKind kind) {
  if (text.empty() || text.size() > 128) return std::nullopt;
  std::vector<Range> result;
  int previous = 899;
  auto number = [](std::string_view part, int& value) {
    if (part.empty() || part.front() < '0' || part.front() > '9') return false;
    const auto parsed = std::from_chars(part.data(), part.data() + part.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == part.data() + part.size();
  };
  while (!text.empty()) {
    const auto comma = text.find(',');
    const auto entry = text.substr(0, comma);
    const auto colon = entry.find(':');
    int upper = 0, value = 0;
    if (colon == std::string_view::npos || !number(entry.substr(0, colon), upper) ||
        !number(entry.substr(colon + 1), value) || upper <= previous || upper > 2100 ||
        (kind == RangeKind::Bandwidth ? value != 10 && value != 20 && value != 40
                                      : value < 0 || value > 4)) return std::nullopt;
    result.push_back({upper, value});
    if (result.size() > 5) return std::nullopt;
    previous = upper;
    if (comma == std::string_view::npos) break;
    text.remove_prefix(comma + 1);
    if (text.empty()) return std::nullopt;
  }
  if (result.back().upper != 2100) return std::nullopt;
  return result;
}

inline std::optional<int> value_for_pwm(std::string_view text, RangeKind kind, int pwm) {
  if (pwm < 900 || pwm > 2100) return std::nullopt;
  const auto ranges = parse_ranges(text, kind);
  if (!ranges) return std::nullopt;
  for (const auto& range : *ranges) {
    if (pwm <= range.upper) return range.value;
  }
  return std::nullopt;
}

// Files predating configurable ranges retain an already enabled RC mapping.
inline const char* initial_ranges(RangeKind kind, bool already_assigned) {
  if (kind == RangeKind::Bandwidth)
    return already_assigned ? legacy_bw_ranges : default_bw_ranges;
  return already_assigned ? legacy_mcs_ranges : default_mcs_ranges;
}

}  // namespace openhd::rc
