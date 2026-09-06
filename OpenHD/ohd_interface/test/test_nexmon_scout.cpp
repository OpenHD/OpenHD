#include "nexmon_scout.h"
#include <cassert>
#include <vector>

int main() {
  std::vector<uint8_t> packet(24+100+4, 0);
  packet[2]=24; packet[4]=0x6f; packet[16]=0x10;
  packet[17]=12; packet[18]=0x6c; packet[19]=0x09;
  openhd::NexmonObservation result;
  assert(openhd::accumulate_nexmon_packet(packet.data(), packet.size(), 2412, result));
  assert(result.foreign_packets == 1 && result.unknown_rate_packets == 0);
  assert(result.decoded_airtime_us == 159); // 104 bytes at 6 Mbps + preamble
  assert(!openhd::accumulate_nexmon_packet(packet.data(), packet.size(), 5180, result));
  assert(result.foreign_packets == 1); // stale channel never enters evidence
  packet[17]=0;
  assert(openhd::accumulate_nexmon_packet(packet.data(), packet.size(), 2412, result));
  assert(result.unknown_rate_packets == 1 && result.decoded_airtime_us == 159);
  packet[16] |= 0x40;
  assert(!openhd::accumulate_nexmon_packet(packet.data(), packet.size(), 2412, result));
  packet[16]=0x10; packet[4]=0xff;
  assert(!openhd::accumulate_nexmon_packet(packet.data(), packet.size(), 2412, result));
  assert(result.malformed_packets == 1);
  for (unsigned size=0; size<24; ++size)
    assert(!openhd::accumulate_nexmon_packet(packet.data(), size, 2412, result));
}
