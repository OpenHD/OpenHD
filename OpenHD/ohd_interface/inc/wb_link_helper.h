//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_

// Helper to fix up / validate wifibroadcast settings given X cards

#include "wb_link_settings.hpp"
#include "openhd-spdlog.hpp"

namespace openhd::wb{

// check if the disable all frequency checks file exists
bool disable_all_frequency_checks();

// fixup any settings coming from a previous use with a different wifi card (e.g. if user swaps around cards)
void fixup_unsupported_settings(openhd::WBStreamsSettingsHolder& settings,
                                std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards,
                                std::shared_ptr<spdlog::logger> m_console);

// returns true if all cards support setting the MCS index
// false otherwise
bool cards_support_setting_mcs_index(const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards);

// returns true if all cards support setting the channel width (otherwise 20Mhz default is fixed (most likely))
// false otherwise
bool cards_support_setting_channel_width(const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards);

// returns true if the given card supports the given frequency, taking into account if the kernel was modifed or not
//bool card_supports_frequency(const WiFiCard& card,bool kernel_supports_extra_channels,int frequency);

bool cards_support_frequency(
    int frequency,
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards,
    const OHDPlatform& platform,
    const std::shared_ptr<spdlog::logger>& m_console);


bool set_frequency_and_channel_width_for_all_cards(uint32_t frequency,uint32_t channel_width,const std::vector<std::shared_ptr<WifiCardHolder>>& broadcast_cards);


}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
