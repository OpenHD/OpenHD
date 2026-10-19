#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "openhd_rtp.h"

int main() {
  std::atomic<unsigned> corrupt{0};
  std::atomic<unsigned> ready{0};
  auto stream = [&](uint8_t value, bool h265) {
    openhd::RTPHelper helper(h265);
    uint16_t sequence = 0;
    helper.set_out_cb([&](const auto& packets) {
      for (const auto& packet : packets) {
        const auto& p = *packet;
        if (p.size() < 15) { ++corrupt; continue; }
        const uint16_t actual = uint16_t(p[2]) * 256 + p[3];
        if (actual != sequence++) ++corrupt;
        const size_t offset = h265 ? (((p[12] >> 1) & 63) == 49 ? 15 : 14)
                                  : ((p[12] & 31) == 28 ? 14 : 13);
        for (size_t i = offset; i < p.size(); ++i)
          if (p[i] != value) { ++corrupt; break; }
      }
    });
    std::vector<uint8_t> nal(32000, value);
    nal[0] = nal[1] = nal[2] = 0; nal[3] = 1; nal[4] = 0x65;
    if (h265) { nal[4] = 0x26; nal[5] = 1; }
    ++ready;
    while (ready.load() != 2) std::this_thread::yield();
    for (int i = 0; i < 5000; ++i) helper.feed_nalu(nal.data(), nal.size());
  };
  std::thread first(stream, 0xaa, false), second(stream, 0x55, false);
  first.join(); second.join();
  ready = 0;
  std::thread third(stream, 0xaa, true), fourth(stream, 0x55, true);
  third.join(); fourth.join();

  openhd::RTPHelper helper(false);
  std::vector<uint32_t> timestamps;
  unsigned callbacks = 0, markers = 0;
  helper.set_out_cb([&](const auto& packets) {
    ++callbacks;
    for (const auto& p : packets) {
      timestamps.push_back(uint32_t((*p)[4]) << 24 | uint32_t((*p)[5]) << 16 |
                           uint32_t((*p)[6]) << 8 | (*p)[7]);
      markers += ((*p)[1] & 128) != 0;
    }
  });
  const uint8_t au[]{
      0,0,0,1,0x67,0x42,0,0x1e,0x95,0xa8,0x28,0x0f,0,0x44,0xfc,0xb8,0x08,0x80,
      0,0,0,1,0x68,0xce,0x06,0xe2,
      0,0,0,1,0x65,0x88,0x84,0x55,
      0,0,0,1,0x65,0x33,0x84,0x55};
  helper.feed_multiple_nalu(au, sizeof(au));
  bool grouped = callbacks == 1 && markers == 1 && timestamps.size() == 4;
  for (auto t : timestamps) grouped = grouped && t == timestamps.front();
  const uint8_t nal[]{0,0,0,1,0x61,0x88,0x84,0x55};
  const auto previous = timestamps.back();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  helper.feed_nalu(nal, sizeof(nal));
  const uint32_t elapsed = timestamps.back() - previous;
  const bool clock_ok = elapsed >= 1800;  // 30 ms is ~2700 ticks, not 30.
  std::cout << "cross-stream corruption=" << corrupt << " grouped_slices="
            << grouped << " RTP_clock_ticks=" << elapsed << '\n';
  return corrupt == 0 && grouped && clock_ok ? 0 : 1;
}
