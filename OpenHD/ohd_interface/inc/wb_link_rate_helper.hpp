//
// Created by consti10 on 12.07.23.
//

#ifndef OPENHD_WB_LINK_RATE_HELPER_HPP
#define OPENHD_WB_LINK_RATE_HELPER_HPP

#include "wifi_card.h"
#include <stdint.h>
//#include "../../lib/wifibroadcast/src/HelperSources/Rates.hpp"

namespace openhd::wb{

// Theoretical rates can be found here: https://mcsindex.com/
// These values are openhd evo specific, since there is more to rates than just the bitrate ;)
struct Rate20Mhz40Mhz{
  uint32_t rate_20mhz;
  uint32_t rate_40mhz;
};
static Rate20Mhz40Mhz rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index) {
    /*if(true){
        auto tmp=wifibroadcast::get_practical_rate_5G(mcs_index);
        return Rate20Mhz40Mhz{(uint32_t)(tmp.rate_20mhz_kbits*100/90),(uint32_t)(tmp.rate_40mhz_kbits*100/90)};
    }*/
  switch (mcs_index) {
    case 0:
      //theoretical:6.5 | 13.5
      // max injection rate possible measured on the bench: 5.7 | 10.4
      // OLD return 4500;
      return {
        5700-1000, // minus 1MBit/s
        10400-3000, // minus 3MBit/s
      };
    case 1:
      //theoretical:13 | 27
      // max injection rate possible measured on the bench: 10.8 | 18.8
      // OLD return 6500;
      return {
        10800-1000, // minus 1MBit/s
        18800-3500, // minus 3.5MBit/s
      };
    case 2:
      //@Norbert: Successfully flown on MCS2 and 7MBit/s video, aka 8.4MBit/s after FEC
      //theoretical:19.5 | 40.5
      // max injection rate possible measured on the bench: 15.2 | 26.6
      // OLD return 8500;
      return {
        15200-2000, // minus 2MBit/s
        26600-4000, // minus 4MBit/s
      };
    case 3:
      //theoretical:26 | 54
      // max injection rate possible measured on the bench: 19.2 | 30+ (out of capabilities of encoder)
      // OLD return 12000;
      return {
        19200-3000, // minus 3MBit/s
        30000-5000, // minus 5MBit/s
      };
    // In general, we only use / recommend MCS 0..3
    case 4:
      //theoretical:39
      return {
         20000, 30000
      };
    case 5:
      //theoretical:52
      return {
        23000, 40000
      };
    case 6:
      //theoretical:58.5
      return {
        26000, 50000
      };
    case 7:
      //theoretical:65
      return {
        29000, 55000
      };
    // MCS 8 == MCS 0 with 2 spatial streams
    case 8:
          // theoretical 13 | 27
          // measured: ~11.7 | 22.1
          return {
                  11700-3000,
                  22100-4000
          };
    case 9:
          // theoretical 26 | 54
          // measured: ~21 | 30+
          return {
                  21000-3000,
                  32000-4000
          };
    case 10:
          // theoretical 39 | 81
          // measured: ~22 | none
          // here we already pretty much reach the limit what encoding hw (rpi) can do
          return {
                  25000-3000,
                  37000-4000
          };
    case 11:
          // theoretical 52 | 108
          return {
                  30000-3000,
                  50000-4000
          };
    case 12:
          // theoretical 78 | 162
          return {
                  30000-3000,
                  50000-4000
          };
    default:
      break;
  }
  return {5000,5000};
}

static uint32_t rtl8812au_get_max_rate_5G_kbits(uint16_t mcs_index,bool is_40_mhz){
  auto rate_kbits= rtl8812au_get_max_rate_5G_kbits(mcs_index);
  return is_40_mhz ? rate_kbits.rate_40mhz : rate_kbits.rate_20mhz;
}

static uint32_t rtl8812au_get_max_rate_2G_kbits(uint16_t mcs_index,bool is_40_mhz){
  const auto rates_5G= rtl8812au_get_max_rate_5G_kbits(mcs_index);
  if(is_40_mhz){
    return rates_5G.rate_40mhz * 60/100;
  }
  return rates_5G.rate_20mhz * 80/100;
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
    if(card.type==WiFiCardType::Atheros9khtc){
        return 9*1000; // About what is fixed in the driver
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
