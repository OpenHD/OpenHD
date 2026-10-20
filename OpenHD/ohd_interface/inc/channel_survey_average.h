#pragma once
#include <cstdint>

namespace openhd::wb {
// Equal observation windows: average successful visits, never treat a failed
// retune or an unsupported measurement as a quiet channel.
struct ChannelSurveyAverage {
  uint64_t packet_sum = 0, energy_sum = 0, busy_sum = 0;
  unsigned packet_count = 0, energy_count = 0, busy_count = 0;
  void add(uint16_t packets, uint16_t metrics) {
    if (metrics == 0x3fff) return;
    packet_sum += packets;
    ++packet_count;
    const auto energy = (metrics >> 7) & 0x7f;
    const auto busy = metrics & 0x7f;
    if ((metrics & 0x8000) && energy <= 100) { energy_sum += energy; ++energy_count; }
    if ((metrics & 0x4000) && busy <= 100) { busy_sum += busy; ++busy_count; }
  }
  uint16_t packets() const {
    return packet_count ? (packet_sum + packet_count / 2) / packet_count : 0;
  }
  uint16_t metrics() const {
    if (!packet_count) return 0x3fff;
    return (energy_count ? 0x8000 | (((energy_sum + energy_count / 2) / energy_count) << 7) : 0) |
           (busy_count ? 0x4000 | ((busy_sum + busy_count / 2) / busy_count) : 0);
  }
};
}
