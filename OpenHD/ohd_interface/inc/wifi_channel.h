//
// Created by consti10 on 11.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_CHANNEL_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_CHANNEL_H_

#include <sstream>
#include <vector>
#include <cstdint>

namespace openhd {

enum class Space { G2_4, G5_8 };

// Wifi channel and the corresponding frequency, in mHz.
// "standard" : listed under wikipedia or not.
struct WifiChannel {
  // frequency in mhz, recommended to use
  uint32_t frequency;
  // channel corresponding to this frequency, error-prone compared to the frequency since often defined incorrectly and/or for the non-standard AR9271 frequencies there is no standard channel number
  int channel;
  // weather this is a channel in the 2.4G space or 5.8G space
  Space space;
  // weather this channel is listed under wikipedia or not.
  // Channels not listed under wikipedia might still work on some cards, given the driver has been modified. generally, they are not legally usable in most countries though.
  bool is_standard;
  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    ss << (int)frequency << "Mhz [" << channel << "] "
       << (space == Space::G2_4 ? "2.4G" : "5.8G")
       << (is_standard ? "" : "nonstandard");
    return ss.str();
  }
};

// These are not valid 2.4G wifi channel(s) but some cards aparently can do them, too From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: In OpenHD we never use the channel number (since it is prone to errors, even in the linux kernel) but rather use the frequency in mhz, which is well-defined. Also read https://yo3iiu.ro/blog/?p=1301
static std::vector<WifiChannel> get_channels_below_standard_2G_wifi() {
  return std::vector<WifiChannel>{
      WifiChannel{2312, -1, Space::G2_4, false},
      WifiChannel{2317, -1, Space::G2_4, false},
      WifiChannel{2322, -1, Space::G2_4, false},
      WifiChannel{2327, -1, Space::G2_4, false},
      WifiChannel{2332, -1, Space::G2_4, false},
      WifiChannel{2337, -1, Space::G2_4, false},
      WifiChannel{2342, -1, Space::G2_4, false},
      WifiChannel{2347, -1, Space::G2_4, false},
      WifiChannel{2352, -1, Space::G2_4, false},
      WifiChannel{2357, -1, Space::G2_4, false},
      WifiChannel{2362, -1, Space::G2_4, false},
      WifiChannel{2367, -1, Space::G2_4, false},
      WifiChannel{2372, -1, Space::G2_4, false},
      WifiChannel{2377, -1, Space::G2_4, false},
      WifiChannel{2382, -1, Space::G2_4, false},
      WifiChannel{2387, -1, Space::G2_4, false},
      WifiChannel{2392, -1, Space::G2_4, false},
      WifiChannel{2397, -1, Space::G2_4, false},
      WifiChannel{2402, -1, Space::G2_4, false},
      WifiChannel{2407, -1, Space::G2_4, false},
  };
}

// https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)
static std::vector<WifiChannel> get_channels_2G_standard() {
  return std::vector<WifiChannel>{
      WifiChannel{2412, 1, Space::G2_4, true},
      WifiChannel{2417, 2, Space::G2_4, true},
      WifiChannel{2422, 3, Space::G2_4, true},
      WifiChannel{2427, 4, Space::G2_4, true},
      WifiChannel{2432, 5, Space::G2_4, true},
      WifiChannel{2437, 6, Space::G2_4, true},
      WifiChannel{2442, 7, Space::G2_4, true},
      WifiChannel{2447, 8, Space::G2_4, true},
      WifiChannel{2452, 9, Space::G2_4, true},
      WifiChannel{2457, 10, Space::G2_4, true},
      WifiChannel{2462, 11, Space::G2_4, true},
      // Temporary disabled - they won't work unil we patch this shit in the kernel
      WifiChannel{2467, 12, Space::G2_4, true},
      WifiChannel{2472, 13, Space::G2_4, true},
      // until here it is consistent (5Mhz increments)
      // this one is neither allowed in EU nor USA
      // (only in Japan under 11b)
      WifiChannel{2484, 14, Space::G2_4, true},
  };
};

// These are not valid 2.4G wifi channel(s) but some cards apparently can do them, too From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: channel and frequency seem to be off by one
static std::vector<WifiChannel> get_channels_above_standard_2G_wifi() {
  return std::vector<WifiChannel>{
      WifiChannel{2487, -1, Space::G2_4, false},
      WifiChannel{2489, -1, Space::G2_4, false},
      WifiChannel{2492, -1, Space::G2_4, false},
      WifiChannel{2494, -1, Space::G2_4, false},
      WifiChannel{2497, -1, Space::G2_4, false},
      WifiChannel{2499, -1, Space::G2_4, false},
      WifiChannel{2512, -1, Space::G2_4, false},
      WifiChannel{2532, -1, Space::G2_4, false},
      WifiChannel{2572, -1, Space::G2_4, false},
      WifiChannel{2592, -1, Space::G2_4, false},
      WifiChannel{2612, -1, Space::G2_4, false},
      WifiChannel{2632, -1, Space::G2_4, false},
      WifiChannel{2652, -1, Space::G2_4, false},
      WifiChannel{2672, -1, Space::G2_4, false},
      WifiChannel{2692, -1, Space::G2_4, false},
      WifiChannel{2712, -1, Space::G2_4, false},
      WifiChannel{2732, -1, Space::G2_4, false},
  };
}

template <class T>
static void vec_append(std::vector<T>& dest, const std::vector<T>& src) {
  dest.insert(dest.end(), src.begin(), src.end());
}

static std::vector<WifiChannel> get_channels_2G(
    const bool include_nonstandard_channels) {
  // keep the order of channels
  std::vector<WifiChannel> ret{};
  if (include_nonstandard_channels) {
    vec_append(ret, get_channels_below_standard_2G_wifi());
  }
  vec_append(ret, get_channels_2G_standard());
  if (include_nonstandard_channels) {
    vec_append(ret, get_channels_above_standard_2G_wifi());
  }
  return ret;
}

// These are not valid 2.4G wifi channel(s) but some cards aparently can do them, too From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: channel and frequency seem to be off by one
static std::vector<WifiChannel> get_channels_5G_below() {
  return std::vector<WifiChannel>{
      WifiChannel{4920, 54, Space::G5_8, false},
      WifiChannel{4940, 55, Space::G5_8, false},
      WifiChannel{4960, 56, Space::G5_8, false},
      WifiChannel{4980, 57, Space::G5_8, false},
  };
}

// https://en.wikipedia.org/wiki/List_of_WLAN_channels#5_GHz_(802.11a/h/j/n/ac/ax)
// These are what iw list lists for rtl8812au
static std::vector<WifiChannel> get_channels_5G_standard() {
  return std::vector<WifiChannel>{
      // TODO {5170,34},
      WifiChannel{5180, 36, Space::G5_8, true},
      WifiChannel{5200, 40, Space::G5_8, true},
      WifiChannel{5220, 44, Space::G5_8, true},
      WifiChannel{5240, 48, Space::G5_8, true},
      WifiChannel{5260, 52, Space::G5_8, true},
      WifiChannel{5280, 56, Space::G5_8, true},
      WifiChannel{5300, 60, Space::G5_8, true},
      WifiChannel{5320, 64, Space::G5_8, true},
      WifiChannel{5500, 100, Space::G5_8, true},
      WifiChannel{5520, 104, Space::G5_8, true},
      WifiChannel{5540, 108, Space::G5_8, true},
      WifiChannel{5560, 112, Space::G5_8, true},
      WifiChannel{5580, 116, Space::G5_8, true},
      WifiChannel{5600, 120, Space::G5_8, true},
      WifiChannel{5620, 124, Space::G5_8, true},
      WifiChannel{5640, 128, Space::G5_8, true},
      WifiChannel{5660, 132, Space::G5_8, true},
      WifiChannel{5680, 136, Space::G5_8, true},
      WifiChannel{5700, 140, Space::G5_8, true},
      WifiChannel{5720, 144, Space::G5_8, true},
      WifiChannel{5745, 149, Space::G5_8, true},
      WifiChannel{5765, 153, Space::G5_8, true},
      WifiChannel{5785, 157, Space::G5_8, true},
      WifiChannel{5805, 161, Space::G5_8, true},
      WifiChannel{5825, 165, Space::G5_8, true},
      WifiChannel{5845, 169, Space::G5_8, true},
      WifiChannel{5865, 173, Space::G5_8, true},
      WifiChannel{5885, 177, Space::G5_8, true},
  };
};

static std::vector<WifiChannel> get_channels_5G(
    const bool include_nonstandard_channels) {
  std::vector<WifiChannel> ret{};
  if (include_nonstandard_channels) {
    vec_append(ret, get_channels_5G_below());
  }
  vec_append(ret, get_channels_5G_standard());
  return ret;
}

static std::vector<WifiChannel> get_all_channels_2G_5G() {
  std::vector<WifiChannel> channels{};
  vec_append(channels, get_channels_2G(true));
  vec_append(channels, get_channels_5G(true));
  return channels;
}

static std::vector<uint32_t> get_all_channel_frequencies(
    const std::vector<openhd::WifiChannel>& channels) {
  std::vector<uint32_t> frequencies;
  for (const auto& channel : channels) {
    frequencies.push_back(channel.frequency);
  }
  return frequencies;
}

// Returns the corresponding Wi-Fi Channel if there is one
// since the mavlink setting is an int, this might not always be possible (and the frequency is then 100% not possible) and therefore should be discarded / fixed
static std::optional<openhd::WifiChannel> channel_from_frequency(uint32_t frequency) {
  const auto channels = get_all_channels_2G_5G();
  for (const auto& channel : channels) {
    if (channel.frequency == frequency) {
      return channel;
    }
  }
  return std::nullopt;
}

static std::vector<openhd::WifiChannel> get_all_channels_from_safe_frequencies(
    const std::vector<uint32_t>& frequencies) {
  std::vector<openhd::WifiChannel> ret;
  for (const auto& freq : frequencies) {
    auto channel= channel_from_frequency(freq);
    assert(channel.has_value());
    ret.push_back(channel.value());
  }
  return ret;
}

}
#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_CHANNEL_H_
