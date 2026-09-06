#pragma once

#include <cstdint>
#include <vector>

namespace openhd {

// On-demand lease of the internal Pi radio. The helper restores its original
// NetworkManager connection and interface name, including after process exit.
class NexmonScout {
 public:
  static bool installed();
  static constexpr const char* monitor_interface = "ohdscout";
  NexmonScout();
  ~NexmonScout();
  NexmonScout(const NexmonScout&) = delete;
  NexmonScout& operator=(const NexmonScout&) = delete;
  bool tune(int frequency_mhz);
  bool restore();
 private:
  bool m_active = false;
};

struct NexmonObservation {
  uint32_t foreign_packets = 0;
  uint64_t decoded_airtime_us = 0;
  uint32_t unknown_rate_packets = 0;
  uint32_t malformed_packets = 0;
};

// Parse only the 24-byte radiotap format emitted by the pinned Nexmon firmware.
// Unknown formats/rates must not become fabricated zero-occupancy evidence.
bool accumulate_nexmon_packet(const uint8_t* data, unsigned size,
                             int frequency_mhz, NexmonObservation& result);
NexmonObservation observe_nexmon(int frequency_mhz, int duration_ms);

}  // namespace openhd
