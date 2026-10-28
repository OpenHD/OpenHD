#pragma once
#include <cmath>
#include <cstdint>
// Shared production rate tables and FEC accounting. No platform/radio
// ownership.
namespace openhd::wb {
// Theoretical rates can be found here: https://mcsindex.com/
// These values are openhd evo specific, since there is more to rates than just
// the bitrate ;)
struct Rate10Mhz20Mhz40Mhz {
  uint32_t rate_10mhz;
  uint32_t rate_20mhz;
  uint32_t rate_40mhz;
};
static Rate10Mhz20Mhz40Mhz rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index) {
  switch (mcs_index) {
    case 0:
      return {
          3200 - 300,   // minus 0.3MBit/s
          5700 - 1000,  // minus 1MBit/s
          10400 - 3000  // minus 3MBit/s
      };
    case 1:
      return {
          5400 - 300,    // minus 0.3MBit/s
          10800 - 1000,  // minus 1MBit/s
          18800 - 3500   // minus 3.5MBit/s
      };
    case 2:
      return {
          8400 - 800,    // minus 0.8MBit/s
          15200 - 2000,  // minus 2MBit/s
          26600 - 6000   // minus 6MBit/s
      };
    case 3:
      return {
          10200 - 800,   // minus 0.8MBit/s
          19200 - 3000,  // minus 3MBit/s
          30000 - 5000   // minus 5MBit/s
      };
    case 4:
      return {
          12000 - 1000,  // minus 1MBit/s
          20000,         //
          30000          //
      };
    case 5:
      return {
          14000 - 1000,  // minus 1MBit/s
          23000,         //
          40000          //
      };
    case 6:
      return {
          16000 - 1000,  // minus 1MBit/s
          26000,         //
          50000          //
      };
    case 7:
      return {
          18000 - 1000,  // minus 1MBit/s
          29000,         //
          55000          //
      };
    case 8:
      return {
          11700 - 3000,  // minus 3MBit/s
          11700 - 3000,  // same as 20MHz
          22100 - 4000   // minus 4MBit/s
      };
    case 9:
      return {
          15000 - 2000,  // minus 2MBit/s
          21000 - 3000,  // minus 3MBit/s
          32000 - 4000   // minus 4MBit/s
      };
    case 10:
      return {
          18000 - 3000,  // minus 3MBit/s
          25000 - 3000,  // minus 3MBit/s
          37000 - 4000   // minus 4MBit/s
      };
    case 11:
      return {
          21000 - 3000,  // minus 3MBit/s
          30000 - 3000,  // minus 3MBit/s
          50000 - 4000   // minus 4MBit/s
      };
    case 12:
      return {
          22000 - 3000,  // minus 3MBit/s
          30000 - 3000,  // minus 3MBit/s
          50000 - 4000   // minus 4MBit/s
      };
    default:
      break;
  }
  return {5000, 5000, 5000};
}

static uint32_t rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index,
                                                int channel_bw_mhz) {
  auto rate_kbits = rtl8812au_get_max_rate_5G_kbits(mcs_index);
  switch (channel_bw_mhz) {
    case 5:
      return rate_kbits.rate_10mhz / 2;
    case 10:
      return rate_kbits.rate_10mhz;
    case 20:
      return rate_kbits.rate_20mhz;
    case 40:
      return rate_kbits.rate_40mhz;
    default:
      return 5000;
  }
}

static int deduce_fec_overhead(int bandwidth_kbits, int fec_overhead_perc) {
  const double tmp = bandwidth_kbits * 100.0 / (100.0 + fec_overhead_perc);
  return static_cast<int>(std::roundl(tmp));
}

static int multiply_by_perc(int bandwidth_kbits, int percentage) {
  return bandwidth_kbits * percentage / 100;
}

inline uint32_t adjust_rate_for_2g(uint32_t rate_5g) {
  // Preserve the existing OpenHD band adjustment exactly.
  return rate_5g * 100 / 80;
}
inline int devourer_payload_budget_kbits(int frequency_mhz, int width_mhz,
                                         int mcs, int fec_percent) {
  auto rate = rtl8812au_get_max_rate_5G_kbits(mcs, width_mhz);
  if (frequency_mhz < 3000) rate = adjust_rate_for_2g(rate);
  return deduce_fec_overhead(rate, fec_percent);
}
}  // namespace openhd::wb
