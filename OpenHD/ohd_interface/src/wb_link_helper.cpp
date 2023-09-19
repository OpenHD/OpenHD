//
// Created by consti10 on 10.12.22.
//

#include "wb_link_helper.h"

#include "wifi_command_helper.h"
//#include "wifi_command_helper2.h"

bool openhd::wb::disable_all_frequency_checks() {
  static constexpr auto FIlE_DISABLE_ALL_FREQUENCY_CHECKS="/boot/openhd/disable_all_frequency_checks.txt";
  return OHDFilesystemUtil::exists(FIlE_DISABLE_ALL_FREQUENCY_CHECKS);
}

bool openhd::wb::all_cards_support_frequency(
    uint32_t frequency,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console=nullptr) {

  // and check if all cards support the frequency
  for(const auto& card:m_broadcast_cards){
    if(!wifi_card_supports_frequency(card,frequency)){
      if(m_console){
        m_console->debug("Card {} doesn't support frequency {}",card.device_name,frequency);
      }
      return false;
    }
  }
  return true;
}

bool openhd::wb::all_cards_support_frequency_and_channel_width(uint32_t frequency, uint32_t channel_width,
                                                               const std::vector<WiFiCard> &m_broadcast_cards,
                                                               const std::shared_ptr<spdlog::logger> &m_console) {
    for(const auto& card:m_broadcast_cards){
        if(!wifi_card_supports_frequency_channel_width(card,frequency,channel_width)){
            if(m_console){
                m_console->debug("Card {} doesn't support frequency/channel width {}:{}",card.device_name,frequency,channel_width);
            }
            return false;
        }
    }
    return true;
}

bool openhd::wb::any_card_support_frequency(uint32_t frequency, const std::vector<WiFiCard> &m_broadcast_cards,
                                            const OHDPlatform &platform,
                                            const std::shared_ptr<spdlog::logger> &m_console) {
    bool any_supports_frequency= false;
    for(const auto& card:m_broadcast_cards){
        if(wifi_card_supports_frequency(card,frequency)){
            any_supports_frequency= true;
        }
    }
    return any_supports_frequency;
}

void openhd::wb::fixup_unsupported_frequency(
    openhd::WBLinkSettingsHolder& settings,
    const std::vector<WiFiCard>& m_broadcast_cards,
    std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console) {
    m_console=openhd::log::get_default();
  }
  // For now, we only check whatever the first card can do and assume the rest can do the same
  const WiFiCard& first_card= m_broadcast_cards.at(0);

  if(!wifi_card_supports_frequency(first_card,settings.get_settings().wb_frequency)){
    m_console->warn("Card {} doesn't support {}Mhz, applying defaults",first_card.device_name,settings.get_settings().wb_frequency);
    if(first_card.supports_5GHz()){
      settings.unsafe_get_settings().wb_frequency=DEFAULT_5GHZ_FREQUENCY;
      settings.persist();
    }else{
      settings.unsafe_get_settings().wb_frequency=DEFAULT_2GHZ_FREQUENCY;
      settings.persist();
    }
  }
  // NOTE: The card might not support setting the MCS index
  /*if(!all_cards_support_setting_mcs_index(m_broadcast_cards)){
    m_console->warn("Card {} doesn't support setting MCS index, applying defaults",first_card.device_name);
    // cards that do not support changing the mcs index are always fixed to mcs3 on openhd drivers
    settings.unsafe_get_settings().wb_air_mcs_index=3;
    settings.persist();
  }*/
  settings.persist();
}

bool openhd::wb::set_frequency_and_channel_width_for_all_cards(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards) {
  bool ret=true;
  for(const auto& card: m_broadcast_cards){
      if(card.type==WiFiCardType::OPENHD_RTL_88X2AU || card.type==WiFiCardType::OPENHD_RTL_88X2BU){
          const int type=card.type==WiFiCardType::OPENHD_RTL_88X2AU ? 0 : 1;
          wifi::commandhelper::openhd_driver_set_frequency_and_channel_width(type,card.device_name, frequency,channel_width);
      }else{
          const bool success=wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,frequency,channel_width);
          //const bool success=wifi::commandhelper2::set_wifi_frequency_and_log_result(card->get_wifi_card().interface_name,frequency,channel_width);
          if(!success){
              ret=false;
          }
      }
  }
  return ret;
}

bool openhd::wb::set_tx_power_for_all_cards(int tx_power_mw, int rtl8812au_tx_power_index_override,
                                            const std::vector <WiFiCard> &m_broadcast_cards) {
    bool ret=true;
    for(const auto& card: m_broadcast_cards){
        if(card.type==WiFiCardType::OPENHD_RTL_88X2AU) {
            openhd::log::get_default()->debug("RTL8812AU tx_pwr_idx_override: {}", rtl8812au_tx_power_index_override);
            wifi::commandhelper::iw_set_tx_power(card.device_name, rtl8812au_tx_power_index_override);
        }else {
            const auto tx_power_mbm=openhd::milli_watt_to_mBm(tx_power_mw);
            openhd::log::get_default()->debug("Tx power mW:{} mBm:{}", tx_power_mw,tx_power_mbm);
            if(card.type==WiFiCardType::OPENHD_RTL_88X2BU) {
                wifi::commandhelper::openhd_driver_set_tx_power(card.device_name,tx_power_mbm);
            }else {
                wifi::commandhelper::iw_set_tx_power(card.device_name,tx_power_mbm);
            }
        }
    }
    return ret;
}

std::vector<std::string> openhd::wb::get_card_names(const std::vector<WiFiCard>& cards) {
  std::vector<std::string> ret{};
  for(const auto& card: cards){
    ret.push_back(card.device_name);
  }
  return ret;
}

bool openhd::wb::any_card_supports_stbc_ldpc_sgi(const std::vector<WiFiCard> &cards) {
    for(const auto& card: cards){
        if(card.type==WiFiCardType::OPENHD_RTL_88X2AU || card.type==WiFiCardType::OPENHD_RTL_88X2BU){
            return true;
        }
    }
    return false;
}

std::vector <openhd::WifiChannel> openhd::wb::get_scan_channels_frequencies(const WiFiCard &card, bool check_2g, bool check_5g) {
    std::vector<openhd::WifiChannel> channels_to_scan;
    if(check_2g && card.supports_2GHz()){
        auto tmp=openhd::get_channels_2G();
        OHDUtil::vec_append(channels_to_scan,tmp);
    }
    if(check_5g && card.supports_5GHz()){
        auto tmp=openhd::get_channels_5G();
        OHDUtil::vec_append(channels_to_scan,tmp);
    }
    return channels_to_scan;
}

std::vector<uint16_t> openhd::wb::get_scan_channels_bandwidths(bool scan_20mhz, bool scan_40mhz) {
    std::vector<uint16_t> channel_widths_to_scan;
    if(scan_20mhz){
        channel_widths_to_scan.push_back(20);
    }
    if(scan_40mhz){
        channel_widths_to_scan.push_back(40);
    }
    return channel_widths_to_scan;
}

std::vector<openhd::WifiChannel> openhd::wb::get_analyze_channels_frequencies(const WiFiCard &card) {
    std::vector<openhd::WifiChannel> channels_to_analyze;
    const auto supported_freq_5G=card.supported_frequencies_5G;
    const auto supported_freq_2G=card.supported_frequencies_2G;
    for(const auto& freq:supported_freq_2G){
        auto tmp=openhd::channel_from_frequency(freq);
        if(tmp.has_value()){
            channels_to_analyze.push_back(tmp.value());
        }
    }
    for(const auto freq:supported_freq_5G){
        auto tmp=openhd::channel_from_frequency(freq);
        if(tmp.has_value()){
            channels_to_analyze.push_back(tmp.value());
        }
    }
    return channels_to_analyze;
}

bool openhd::wb::has_any_rtl8812au(const std::vector<WiFiCard>& cards) {
    for(const auto& card: cards){
        if(card.type==WiFiCardType::OPENHD_RTL_88X2AU){
            return true;
        }
    }
    return false;
}

bool openhd::wb::has_any_non_rtl8812au(const std::vector<WiFiCard> &cards) {
    for(const auto& card: cards){
        if(card.type!=WiFiCardType::OPENHD_RTL_88X2AU){
            return true;
        }
    }
    return false;
}

