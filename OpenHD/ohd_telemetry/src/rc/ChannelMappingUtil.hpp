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

#ifndef OPENHD_CHANNELMAPPINGUTIL_H
#define OPENHD_CHANNELMAPPINGUTIL_H

#include <array>
#include <charconv>
#include <cstdint>
#include <optional>
#include <string>


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

// Each output selects a raw input; -1 (stored as 0) leaves the output unassigned.
using CHAN_MAP = std::array<int, N_MAPPED_CHANNELS>;

static bool validate_channel_mapping(const CHAN_MAP& chan_map) {
  for (const auto& el : chan_map) {  // NOLINT(readability-use-anyofallof)
    if (el < -1 || el >= N_MAV_CHANNELS) {
      return false;
    }
  }
  return true;
}

static std::optional<CHAN_MAP> convert_string_to_channel_mapping(
    const std::string& input) {
  CHAN_MAP parsed_as_int{};
  const char* begin = input.data();
  const char* end = begin + input.size();
  for (int i = 0; i < N_MAPPED_CHANNELS; ++i) {
    int value = -1;
    auto parsed = std::from_chars(begin, end, value);
    if (parsed.ec != std::errc{} || parsed.ptr == begin || value < 0 || value > 18)
      return std::nullopt;
    parsed_as_int[i] = value - 1;
    if (i == N_MAPPED_CHANNELS - 1) {
      if (parsed.ptr != end) return std::nullopt;
    } else {
      if (parsed.ptr == end || *parsed.ptr != ',') return std::nullopt;
      begin = parsed.ptr + 1;
    }
  }
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
      if (channel_to_use == -1) {
        // MAVLink uses different release values for channels 1-8 and 9-18.
        // Release a previous assignment instead of leaving a stale override active.
        ret[i] = i < 8 ? 0 : UINT16_MAX - 1;
      } else if (channel_to_use >= 0 && channel_to_use < channels.size()) {
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
