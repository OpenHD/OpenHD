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

#ifndef OPENHD_WB_LINK_RATE_HELPER_HPP
#define OPENHD_WB_LINK_RATE_HELPER_HPP

#include <stdint.h>

#include "wifi_card.h"

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

static Rate10Mhz20Mhz40Mhz rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index) {
  switch (mcs_index) {
    case 0:
      return {
          3600 - 1000,  // minus 1MBit/s
          4600 - 1000,  // minus 1MBit/s
          6500 - 2000   // minus 2MBit/s
      };
    case 1:
      return {
          8200 - 1000,   // minus 1MBit/s
          10100 - 1000,  // minus 1MBit/s
          15900 - 2000   // minus 2MBit/s
      };
    case 2:
      return {
          10200 - 1500,  // minus 1.5MBit/s
          13500 - 2000,  // minus 2MBit/s
          20000 - 2000   // minus 2MBit/s
      };
    case 3:
      return {
          12800 - 2000,  // minus 2MBit/s
          16600 - 2000,  // minus 2MBit/s
          24000 - 2000   // minus 2MBit/s
      };
    case 4:
      return {
          15000,  //
          20000,  //
          30000   //
      };
    default: {
      openhd::log::get_default()->warn("MCS >4 not recommended");
      return {15000, 20000, 30000};
    }
  }
  assert(false);
}

static uint32_t rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index,
                                                int channel_bw_mhz) {
  auto rate_kbits = rtl8812au_get_max_rate_5G_kbits(mcs_index);
  switch (channel_bw_mhz) {
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

static uint32_t rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index,
                                                int channel_bw_mhz) {
  auto rate_kbits = rtl8812au_get_max_rate_2G_kbits(mcs_index);
  switch (channel_bw_mhz) {
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

static uint32_t get_max_rate_possible_5G_kbits(const WiFiCard& card,
                                               uint16_t mcs_index,
                                               int channel_bw_mhz) {
  if (card.type == WiFiCardType::OPENHD_RTL_88X2AU ||
      card.type == WiFiCardType::OPENHD_RTL_88X2BU ||
      card.type == WiFiCardType::OPENHD_RTL_88X2CU ||
      card.type == WiFiCardType::OPENHD_RTL_88X2EU ||
      card.type == WiFiCardType::OPENHD_RTL_8852BU ||
      card.type == WiFiCardType::OPENHD_EMULATED) {
    return rtl8812au_get_max_rate_5G_kbits(mcs_index, channel_bw_mhz);
  }
  return 5000;
}

static uint32_t get_max_rate_possible_2G_kbits(const WiFiCard& card,
                                               uint16_t mcs_index,
                                               int channel_bw_mhz) {
  const auto rate_5G =
      get_max_rate_possible_5G_kbits(card, mcs_index, channel_bw_mhz);
  // 2.4G is (always) quite crowded, so use less bitrate
  return rate_5G * 100 / 80;
}

static uint32_t get_max_rate_possible(const WiFiCard& card,
                                      const openhd::WifiSpace wifi_space,
                                      uint16_t mcs_index, int channel_bw_mhz) {
  if (wifi_space == WifiSpace::G2_4) {
    return get_max_rate_possible_2G_kbits(card, mcs_index, channel_bw_mhz);
  }
  assert(wifi_space == WifiSpace::G5_8);
  return get_max_rate_possible_5G_kbits(card, mcs_index, channel_bw_mhz);
}

static int deduce_fec_overhead(int bandwidth_kbits, int fec_overhead_perc) {
  const double tmp = bandwidth_kbits * 100.0 / (100.0 + fec_overhead_perc);
  return static_cast<int>(std::roundl(tmp));
}

static int multiply_by_perc(int bandwidth_kbits, int percentage) {
  return bandwidth_kbits * percentage / 100;
}

}  // namespace openhd::wb

#endif  // OPENHD_WB_LINK_RATE_HELPER_HPP
