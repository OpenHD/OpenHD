#pragma once

#include <array>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace openhd {

enum class RcLinkFunction { Mcs, Bandwidth };

class RcChannelRanges {
 public:
  static std::optional<RcChannelRanges> parse(RcLinkFunction function,
                                              std::string_view text) {
    RcChannelRanges ranges;
    int previous_bound = 899;
    while (!text.empty()) {
      if (ranges.m_count == ranges.m_areas.size()) return std::nullopt;
      const auto comma = text.find(',');
      const auto area = text.substr(0, comma);
      const auto colon = area.find(':');
      if (colon == std::string_view::npos) return std::nullopt;

      const auto bound = parse_number(area.substr(0, colon));
      const auto value = parse_number(area.substr(colon + 1));
      if (!bound || *bound <= previous_bound || *bound > 2100 || !value) {
        return std::nullopt;
      }
      const bool valid_value = function == RcLinkFunction::Mcs
                                   ? *value >= 0 && *value <= 4
                                   : *value == 10 || *value == 20 || *value == 40;
      if (!valid_value) return std::nullopt;

      ranges.m_areas[ranges.m_count++] = {*bound, *value};
      previous_bound = *bound;
      if (comma == std::string_view::npos) break;
      text.remove_prefix(comma + 1);
      if (text.empty()) return std::nullopt;
    }
    if (previous_bound != 2100) return std::nullopt;
    return ranges;
  }

  std::optional<int> value_for_pwm(int pwm) const {
    if (pwm < 900 || pwm > 2100) return std::nullopt;
    for (std::size_t i = 0; i < m_count; ++i) {
      if (pwm <= m_areas[i].first) return m_areas[i].second;
    }
    return std::nullopt;
  }

  std::string encode() const {
    std::string text;
    for (std::size_t i = 0; i < m_count; ++i) {
      if (i != 0) text += ',';
      text += std::to_string(m_areas[i].first) + ':' +
              std::to_string(m_areas[i].second);
    }
    return text;
  }

 private:
  static std::optional<int> parse_number(std::string_view text) {
    if (text.empty() || text.front() < '0' || text.front() > '9') {
      return std::nullopt;
    }
    int value = 0;
    const auto end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    if (result.ec != std::errc{} || result.ptr != end) return std::nullopt;
    return value;
  }

  std::array<std::pair<int, int>, 5> m_areas{};
  std::size_t m_count = 0;
};

}  // namespace openhd
