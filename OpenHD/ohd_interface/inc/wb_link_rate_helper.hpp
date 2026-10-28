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
#include "wb_link_rate_budget.h"

namespace openhd::wb {

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

static uint32_t rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index,
                                                int channel_bw_mhz) {
  auto rate_kbits = rtl8812au_get_max_rate_2G_kbits(mcs_index);
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

static uint32_t get_max_rate_possible_5G_kbits(const WiFiCard& card,
                                               uint16_t mcs_index,
                                               int channel_bw_mhz) {
  if (card.devourer_wb_enabled ||
      card.type == WiFiCardType::OPENHD_RTL_88X2AU ||
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
  return adjust_rate_for_2g(rate_5G);
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

}  // namespace openhd::wb

#endif  // OPENHD_WB_LINK_RATE_HELPER_HPP
