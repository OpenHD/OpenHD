//
// Created by consti10 on 10.12.22.
//

#include "wb_link_helper.h"

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

bool openhd::wb::card_supports_frequency(const WiFiCard& card,
                                         bool kernel_supports_extra_channels,
                                         int frequency) {
  if(card.supports_2ghz && openhd::is_valid_frequency_2G(frequency,kernel_supports_extra_channels)){
    return true;
  }
  if(card.supports_5ghz && openhd::is_valid_frequency_5G(frequency,kernel_supports_extra_channels)){
    return true;
  }
  return false;
}

bool openhd::wb::cards_support_frequency(
    int frequency,
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards,
    const OHDPlatform& platform,
    const std::shared_ptr<spdlog::logger>& m_console) {

  // check if the frequency is a valid 2G or 5G frequency (we only have 2G or 5G wifi cards)
  if(! (openhd::is_valid_frequency_5G(frequency, true)
        || openhd::is_valid_frequency_2G(frequency,true))){
    m_console->debug("Not a valid 2G or 5G frequency {}",frequency);
    return false;
  }

  // check if we are running on a modified kernel, in which case we can do those extra frequencies that
  // are illegal in most countries (otherwise they are disabled)
  // NOTE: When running on RPI or Jetson we assume we are running on an OpenHD image which has the modified kernel
  const bool kernel_supports_extra_channels=platform.platform_type==PlatformType::RaspberryPi ||
                                              platform.platform_type==PlatformType::Jetson;

  // and check if all cards support the frequency
  for(const auto& card_holder:m_broadcast_cards){
    const auto& card=card_holder->get_wifi_card();
    if(!card_supports_frequency(card,kernel_supports_extra_channels,frequency)){
      m_console->debug("Card {} doesn't support frequency {}",card.interface_name,frequency);
      return false;
    }
  }

  return true;
}

