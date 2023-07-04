//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_

#include "openhd_spdlog.h"
#include "wb_link_settings.hpp"

/**
 * The wb_link class is becoming a bit big and therefore hard to read.
 * Here we have some common helper methods used in wb_link
 */
namespace openhd::wb{

/**
 * @return true if the "disable all frequency checks" file exists
 */
bool disable_all_frequency_checks();

/**
 * @param m_broadcast_cards the cards to check capabilities from
 * @return  true if all cards support setting the channel width (otherwise 20Mhz default is fixed (most likely))
 */
bool cards_support_setting_channel_width(const std::vector<WiFiCard>& m_broadcast_cards);

/**
 * returns true if the given cards supports the given frequency, taking into account if the kernel was modified or not
 */
bool cards_support_frequency(
    uint32_t frequency,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const OHDPlatform& platform,
    const std::shared_ptr<spdlog::logger>& m_console);

/**
 * fixup any settings coming from a previous use with a different wifi card (e.g. if user swaps around cards)
 */
void fixup_unsupported_settings(openhd::WBStreamsSettingsHolder& settings,
                                const std::vector<WiFiCard>& m_broadcast_cards,
                                std::shared_ptr<spdlog::logger> m_console);

bool set_frequency_and_channel_width_for_all_cards(uint32_t frequency,uint32_t channel_width,const std::vector<WiFiCard>& m_broadcast_cards);

// Return some measured rate for a given mcs index on rtl8812au (5.8G and 20Mhz channel width)
// measured means: I (not scientifically) looked at how much I can inject on the bench in a medium/low rf environment before I get tx errors
uint32_t rtl8812au_get_measured_max_rate(uint32_t mcs_index);

// WB takes a list of card device names
std::vector<std::string> get_card_names(const std::vector<WiFiCard>& cards);

// Returns true if any of the given cards is of type rtl8812au
bool has_any_rtl8812au(const std::vector<WiFiCard>& cards);

// Theoretical rates can be found here: https://mcsindex.com/
// Those values are hella conservative, but mostly taken from 2.0
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

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
