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

// return true if the "disable all frequency checks" file exists
bool disable_all_frequency_checks();

// returns true if all cards support setting an MCS index
// false otherwise
bool cards_support_setting_mcs_index(const std::vector<WiFiCard>& m_broadcast_cards);

// returns true if all cards support setting the channel width (otherwise 20Mhz default is fixed (most likely))
// false otherwise
bool cards_support_setting_channel_width(const std::vector<WiFiCard>& m_broadcast_cards);

// returns true if the given card supports the given frequency, taking into account if the kernel was modified or not
bool cards_support_frequency(
    uint32_t frequency,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const OHDPlatform& platform,
    const std::shared_ptr<spdlog::logger>& m_console);

// fixup any settings coming from a previous use with a different wifi card (e.g. if user swaps around cards)
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
}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
