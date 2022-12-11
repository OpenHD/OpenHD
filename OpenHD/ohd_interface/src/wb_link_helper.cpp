//
// Created by consti10 on 10.12.22.
//

#include "wb_link_helper.h"

bool openhd::wb::disable_all_frequency_checks() {
  static constexpr auto FIlE_DISABLE_ALL_FREQUENCY_CHECKS="/boot/openhd/disable_all_frequency_checks.txt";
  return OHDFilesystemUtil::exists(FIlE_DISABLE_ALL_FREQUENCY_CHECKS);
}

void openhd::wb::fixup_unsupported_settings(
    openhd::WBStreamsSettingsHolder& settings,
    std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards,
    std::shared_ptr<spdlog::logger> m_console) {
  // check if the cards connected match the previous settings.
  // For now, we check if the first wb card can do 2 / 4 ghz, and assume the rest can do the same
  const auto first_card= m_broadcast_cards.at(0)->_wifi_card;
  if(settings.get_settings().configured_for_2G()){
    if(! first_card.supports_2ghz){
      // we need to switch to 5ghz, since the connected card cannot do 2ghz
      m_console->warn("WB configured for 2G but card can only do 5G - overwriting old settings");
      settings.set_default_5G();
    }
  }else{
    if(!first_card.supports_5ghz){
      // similar, we need to switch to 2G
      m_console->warn("WB configured for 5G but card can only do 2G - overwriting old settings");
      settings.set_default_2G();
    }
  }
  if(!cards_support_setting_mcs_index(m_broadcast_cards)){
    // cards that do not support changing the mcs index are always fixed to mcs3 on openhd drivers
    settings.unsafe_get_settings().wb_mcs_index=3;
    settings.persist();
  }
  settings.persist();
}

bool openhd::wb::cards_support_setting_channel_width(
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_variable_mcs(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}

bool openhd::wb::cards_support_setting_mcs_index(
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_40Mhz_channel_width(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}

/*bool openhd::wb::card_supports_frequency(const WiFiCard& card,
                                         bool kernel_supports_extra_channels,
                                         int frequency) {
  if(card.supports_2ghz){
    // special and only AR9271: channels below and above standard wifi
    const bool include_extra_channels_2G=kernel_supports_extra_channels && wifi_card_supports_extra_channels_2G(card);
    if(openhd::is_valid_frequency_2G(frequency,include_extra_channels_2G)){
      return true;
    }
  }
  if(card.supports_5ghz && openhd::is_valid_frequency_5G(frequency,kernel_supports_extra_channels)){
    return true;
  }
  return false;
}*/

bool openhd::wb::cards_support_frequency(
    int frequency,
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards,
    const OHDPlatform& platform,
    const std::shared_ptr<spdlog::logger>& m_console=nullptr) {

  // and check if all cards support the frequency
  for(const auto& card_holder:m_broadcast_cards){
    const auto& card=card_holder->get_wifi_card();
    if(!wifi_card_supports_frequency(platform,card,frequency)){
      if(m_console){
        m_console->debug("Card {} doesn't support frequency {}",card.interface_name,frequency);
      }
      return false;
    }
  }
  return true;
}
