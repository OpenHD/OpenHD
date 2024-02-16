#include "mavlink_channels.h"

namespace mavsdk {

MAVLinkChannels::MAVLinkChannels() : _channels_used{}, _channels_used_mutex() {}

bool MAVLinkChannels::checkout_free_channel(uint8_t& new_channel) {
  std::lock_guard<std::mutex> lock(_channels_used_mutex);

  for (unsigned i = 0; i < MAX_CHANNELS; ++i) {
    if (!_channels_used[i]) {
      _channels_used[i] = true;
      new_channel = i;
      return true;
    }
  }
  return false;
}

void MAVLinkChannels::checkin_used_channel(uint8_t used_channel) {
  std::lock_guard<std::mutex> lock(_channels_used_mutex);

  if (used_channel >= MAX_CHANNELS) {
    return;
  }

  _channels_used[used_channel] = false;
}

}  // namespace mavsdk
