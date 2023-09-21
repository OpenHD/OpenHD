//
// Created by consti10 on 10.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_

#include "openhd_spdlog.h"
#include "wb_link_settings.h"
#include <optional>
#include <mutex>

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
 * returns true if all the given cards supports the given frequency
 */
bool all_cards_support_frequency(
    uint32_t frequency,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console);

bool all_cards_support_frequency_and_channel_width(uint32_t frequency,
                                                   uint32_t channel_width,
                                                   const std::vector<WiFiCard>& m_broadcast_cards,
                                                   const std::shared_ptr<spdlog::logger>& m_console);

bool validate_frequency_change(int new_frequency,int current_channel_width,
                               const std::vector<WiFiCard>& m_broadcast_cards,
                               const std::shared_ptr<spdlog::logger>& m_console);
bool validate_air_channel_width_change(int new_channel_width,const WiFiCard& card,
                                       const std::shared_ptr<spdlog::logger>& m_console);

bool validate_air_mcs_index_change(int new_mcs_index,const WiFiCard& card,
                                   const std::shared_ptr<spdlog::logger>& m_console);

bool any_card_support_frequency(
        uint32_t frequency,
        const std::vector<WiFiCard>& m_broadcast_cards,
        const OHDPlatform& platform,
        const std::shared_ptr<spdlog::logger>& m_console);
/**
 * Applies a known working frequency for the given card(s) if none of the card(s) supports the current frequency
 * (E.g. if a user was to swap a 5.8G card for a 2.4G card)
 */
void fixup_unsupported_frequency(openhd::WBLinkSettingsHolder& settings,
                                const std::vector<WiFiCard>& m_broadcast_cards,
                                std::shared_ptr<spdlog::logger> m_console);

bool set_frequency_and_channel_width_for_all_cards(uint32_t frequency,uint32_t channel_width,const std::vector<WiFiCard>& m_broadcast_cards);

bool set_tx_power_for_all_cards(int tx_power_mw,int rtl8812au_tx_power_index_override,const std::vector<WiFiCard>& m_broadcast_cards);


// WB takes a list of card device names
std::vector<std::string> get_card_names(const std::vector<WiFiCard>& cards);

// Returns true if any of the given cards is of type rtl8812au
bool has_any_rtl8812au(const std::vector<WiFiCard>& cards);
// Returns true if any of the given cards is not of type rtl8812au
bool has_any_non_rtl8812au(const std::vector<WiFiCard>& cards);

bool any_card_supports_stbc_ldpc_sgi(const std::vector<WiFiCard>& cards);

std::vector<WifiChannel> get_scan_channels_frequencies(const WiFiCard& card,bool check_2g,bool check_5g);
std::vector<uint16_t> get_scan_channels_bandwidths(bool scan_20mhz,bool scan_40mhz);

std::vector<WifiChannel> get_analyze_channels_frequencies(const WiFiCard& card);

/*
 * Removes network manager from the given cards (if it is running)
 * and in general tries to make sure no linux stuff that would interfer with monitor mode is running on the card(s),
 * and then sets them into monitor mode.
 */
void takeover_cards_monitor_mode(const std::vector<WiFiCard>& cards,std::shared_ptr<spdlog::logger> console);
/**
 * Gives the card(s) back to network manager;
 */
void giveback_cards_monitor_mode(const std::vector<WiFiCard>& cards,std::shared_ptr<spdlog::logger> console);

class ForeignPacketsHelper{
public:
    int update(uint64_t count_p_any,uint64_t count_p_valid){
        const uint64_t n_foreign_packets=count_p_any>count_p_valid ? count_p_any-count_p_valid : 0;
        if(m_foreign_packets_last_time>n_foreign_packets){
            m_foreign_packets_last_time=n_foreign_packets;
            return 0;
        }
        const int delta=static_cast<int>(n_foreign_packets-m_foreign_packets_last_time);
        m_foreign_packets_last_time=n_foreign_packets;
        return delta;
    }
private:
    uint64_t m_foreign_packets_last_time=0;
};

/**
 * This class basically only offers atomic read / write operations on the "RC CHANNELS" as reported by the FC.
 * This is needed for the "MCS VIA RC CHANNEL CHANGE" feature.
 */
class RCChannelHelper{
public:
    void set_rc_channels(const std::array<int, 18>& rc_channels){
        std::lock_guard<std::mutex> guard(m_rc_channels_mutex);
        m_rc_channels=rc_channels;
    }
    std::optional<std::array<int, 18>> get_fc_reported_rc_channels(){
        std::lock_guard<std::mutex> guard(m_rc_channels_mutex);
        return m_rc_channels;
    }
    /**
     * Get the mcs index mapping pwm channel (channel_index) to mcs indices.
     * If no rc data has been supplied by the FC yet and / or the channel index is invalid
     * or the pwm value is not valid, return std::nullopt.
     */
    std::optional<int> get_mcs_from_rc_channel(int channel_index,std::shared_ptr<spdlog::logger>& m_console);
private:
    std::optional<std::array<int, 18>> m_rc_channels;
    std::mutex m_rc_channels_mutex;
};
}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_HELPER_H_
