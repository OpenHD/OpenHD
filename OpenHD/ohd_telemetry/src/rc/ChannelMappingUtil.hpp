//
// Created by consti10 on 06.05.23.
//

#ifndef OPENHD_CHANNELMAPPINGUTIL_H
#define OPENHD_CHANNELMAPPINGUTIL_H

#include <array>
#include <optional>
#include <string>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"

// Util methods for simple channel mapping, where each channel's input can be
// defined by the user.
namespace openhd {

// mavlink rc has 18 channels (for whatever reason)
static constexpr auto N_MAV_CHANNELS = 18;

// REMOVED- We only map the first X channel(s) to keep things simple
// (Ardupilot doesn't do more than 16 channels anyway)
// -> whatever, makes it less complicated to program (but the string gets quite
// big)
static constexpr auto N_MAPPED_CHANNELS = 18;

// Channel mapping: just look at the default to understand ;)
using CHAN_MAP = std::array<int, N_MAPPED_CHANNELS>;

static bool validate_channel_mapping(const CHAN_MAP& chan_map) {
  for (const auto& el : chan_map) {  // NOLINT(readability-use-anyofallof)
    if (el < 0 || el >= N_MAV_CHANNELS) {
      openhd::log::get_default()->warn("Channel mapping not a valid value{}",
                                       el);
      return false;
    }
  }
  return true;
}

static std::optional<CHAN_MAP> convert_string_to_channel_mapping(
    const std::string& input) {
  auto split_into_substrings = OHDUtil::split_into_substrings(input, ',');
  if (split_into_substrings.size() != N_MAPPED_CHANNELS) {
    openhd::log::get_default()->warn("Channel mapping wrong n channels:{}",
                                     split_into_substrings.size());
    return std::nullopt;
  }
  CHAN_MAP parsed_as_int{};
  for (int i = 0; i < N_MAPPED_CHANNELS; i++) {
    // In the stored string, we start counting from 1, but in c++, we
    // use an array index notation.
    const auto as_int = OHDUtil::string_to_int(split_into_substrings[i]);
    if (!as_int.has_value()) return std::nullopt;
    parsed_as_int[i] = as_int.value() - 1;  // array notation
  }
  if (!validate_channel_mapping(parsed_as_int)) return std::nullopt;
  return parsed_as_int;
}

static CHAN_MAP get_default_channel_mapping() {
  CHAN_MAP ret{};
  for (int i = 0; i < N_MAPPED_CHANNELS; i++) {
    ret[i] = i;
  }
  return ret;
}

static CHAN_MAP convert_string_to_channel_mapping_or_default(
    const std::string& input) {
  auto ret = convert_string_to_channel_mapping(input);
  if (ret.has_value()) {
    return ret.value();
  }
  openhd::log::get_default()->warn("Invalid channel mapping [{}],using default",
                                   input);
  return get_default_channel_mapping();
}

static std::array<uint16_t, N_MAV_CHANNELS> remap_channels(
    const std::array<uint16_t, N_MAV_CHANNELS>& channels,
    const CHAN_MAP& chan_map) {
  std::array<uint16_t, N_MAV_CHANNELS> ret{};
  for (int i = 0; i < N_MAV_CHANNELS; i++) {
    // Better be safe than sorry regarding bounds checking (even though we
    // shouldn't ever be out of bounds)
    if (i < chan_map.size()) {
      const auto channel_to_use = chan_map[i];
      if (channel_to_use >= 0 && channel_to_use < channels.size()) {
        ret[i] = channels[channel_to_use];
      }
    } else {
      ret[i] = channels[i];
    }
  }
  return ret;
}

}  // namespace openhd
#endif  // OPENHD_CHANNELMAPPINGUTIL_H
