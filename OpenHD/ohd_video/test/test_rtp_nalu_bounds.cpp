#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "openhd_rtp.h"

// End the final NAL exactly at a protected page. A splitter that reuses the
// original AU length after advancing its pointer reads into the guard page.
int main() {
  const uint8_t access_unit[]{
      0, 0, 0, 1, 0x67, 0x42, 0, 0x1e, 0x95, 0xa8, 0x28, 0x0f,
      0, 0x44, 0xfc, 0xb8, 0x08, 0x80,
      0, 0, 0, 1, 0x68, 0xce, 0x06, 0xe2,
      0, 0, 0, 1, 0x65, 0x88, 0x84, 0x55};
  const auto page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
  auto* allocation = static_cast<uint8_t*>(mmap(
      nullptr, page * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (allocation == MAP_FAILED) return 2;
  if (mprotect(allocation + page, page, PROT_NONE) < 0) return 2;
  auto* data = allocation + page - sizeof(access_unit);
  std::memcpy(data, access_unit, sizeof(access_unit));
  unsigned callbacks = 0;
  bool sps = false, pps = false, picture = false, headers_first = true;
  openhd::RTPHelper packetizer(false);
  packetizer.set_out_cb([&](const auto& packets) {
    if (!packets.empty()) ++callbacks;
    for (const auto& packet : packets) {
      if (packet->size() < 13) { headers_first = false; continue; }
      const int type = (*packet)[12] & 31;
      if (type == 7) sps = true;
      if (type == 8) pps = true;
      if (type >= 1 && type <= 5) {
        picture = true;
        headers_first = headers_first && sps && pps;
      }
    }
  });
  packetizer.feed_multiple_nalu(data, sizeof(access_unit));
  munmap(allocation, page * 2);
  std::cout << "bounded multi-NAL packetization callbacks=" << callbacks << '\n';
  return callbacks > 0 && picture && headers_first ? 0 : 1;
}
