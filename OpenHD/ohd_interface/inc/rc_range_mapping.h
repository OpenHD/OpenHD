#pragma once

#include <charconv>
#include <optional>
#include <string_view>
#include <vector>

namespace openhd::rc {

inline constexpr auto default_bw_ranges = "1500:20,2100:40";
inline constexpr auto default_mcs_ranges = "1300:0,1700:2,2100:3";
inline constexpr auto default_tx_mode_ranges = "1300:0,1700:1,2100:2";
inline constexpr auto default_fhss_ranges = "1500:0,2100:1";
inline constexpr auto legacy_bw_ranges = "1300:10,1700:20,2100:40";
inline constexpr auto legacy_mcs_ranges = "1200:0,1400:1,1600:2,1800:3,2100:4";

enum class RangeKind { Bandwidth, Mcs, TxMode, Fhss };
struct Range {
  int upper;
  int value;
};

inline bool valid_value(RangeKind kind, int value) {
  switch (kind) {
    case RangeKind::Bandwidth: return value == 10 || value == 20 || value == 40;
    case RangeKind::Mcs: return value >= 0 && value <= 4;
    case RangeKind::TxMode: return value >= 0 && value <= 2;
    case RangeKind::Fhss: return value == 0 || value == 1;
  }
  return false;
}

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
        !valid_value(kind, value)) return std::nullopt;
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
  switch (kind) {
    case RangeKind::Bandwidth: return already_assigned ? legacy_bw_ranges : default_bw_ranges;
    case RangeKind::Mcs: return already_assigned ? legacy_mcs_ranges : default_mcs_ranges;
    case RangeKind::TxMode: return default_tx_mode_ranges;
    case RangeKind::Fhss: return default_fhss_ranges;
  }
  return "";
}

}  // namespace openhd::rc
