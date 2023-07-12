//
// Created by consti10 on 12.07.23.
//

#ifndef OPENHD_WB_LINK_RATE_HELPER_HPP
#define OPENHD_WB_LINK_RATE_HELPER_HPP

#include "wifi_card.h"

namespace openhd::wb{

// Theoretical rates can be found here: https://mcsindex.com/
// These values are openhd evo specific, since there is more to rates than just the bitrate ;)
static uint32_t rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index) {
  switch (mcs_index) {
    case 0:
      //theoretical:6.5
      // max injection rate possible measured on the bench: 5.7
      // OLD return 4500;
      return 4700; // minus 1MBit/s
    case 1:
      //theoretical:13
      // max injection rate possible measured on the bench: 10.8
      // OLD return 6500;
      return 9800; // minus 1MBit/s
    case 2:
      //@Norbert: Successfully flown on MCS2 and 7MBit/s video, aka 8.4MBit/s after FEC
      //theoretical:19.5
      // max injection rate possible measured on the bench: 15.2
      // OLD return 8500;
      return 13200; // minus 2MBit/s
    case 3:
      //theoretical:26
      // max injection rate possible measured on the bench: 19.2
      // OLD return 12000;
      return 16200; // minus 3MBit/s
    // In general, we only use / recommend MCS 0..3
    case 4:
      return 17000; //theoretical:39
    case 5:
      return 23000; //theoretical:52
    case 6:
      return 36000; //theoretical:58.5
    case 7:
      return 36000; //theoretical:65
    default:
      break;
  }
  return 5000;
}

static uint32_t rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index,bool is_40_mhz){
  auto rate_kbits= rtl8812au_get_max_rate_5G_kbits(mcs_index);
  if(is_40_mhz){
    // Theoretically 40Mhz should give exactly 2x the bitrate, but be a bit conservative here
    rate_kbits = static_cast<uint32_t>(std::roundl(rate_kbits*1.7));
  }
  return rate_kbits;
}

static uint32_t rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index) {
  // dirty, but 2G in general sucks
  return rtl8812au_get_max_rate_5G_kbits(mcs_index) * 80/100;
}

static uint32_t rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index,bool is_40_mhz){
  auto rate_kbits= rtl8812au_get_max_rate_2G_kbits(mcs_index);
  if(is_40_mhz){
    // Theoretically 40Mhz should give exactly 2x the bitrate, but be a bit conservative here
    rate_kbits = static_cast<uint32_t>(std::roundl(rate_kbits*1.7));
  }
  return rate_kbits;
}

static uint32_t get_max_rate_possible_5G(const WiFiCard& card,uint16_t mcs_index,bool is_40Mhz){
  if(card.type==WiFiCardType::Realtek8812au){
    return rtl8812au_get_max_rate_5G_kbits(mcs_index, is_40Mhz);
  }
  if(card.type==WiFiCardType::Realtek88x2bu){
    // Hard coded, since "some rate" is hard coded in the driver.
    return 10000;
  }
  // fallback for any other weak crap
  return 5000;
}

static uint32_t get_max_rate_possible(const WiFiCard& card,const openhd::WifiSpace wifi_space,uint16_t mcs_index,bool is_40Mhz){
  // Generally, 2G just sucks - we do not have proper rate control for it
  if(wifi_space==WifiSpace::G2_4){
    if(card.type==WiFiCardType::Realtek8812au){
      return rtl8812au_get_max_rate_2G_kbits(mcs_index,is_40Mhz);
    }
    return 5000;
  }
  return get_max_rate_possible_5G(card,mcs_index,is_40Mhz);
}

static uint32_t deduce_fec_overhead(uint32_t bandwidth_kbits,int fec_overhead_perc){
  const double tmp = bandwidth_kbits * 100.0 / (100.0 + fec_overhead_perc);
  return static_cast<uint32_t>(std::roundl(tmp));
}


}
#endif  // OPENHD_WB_LINK_RATE_HELPER_HPP
