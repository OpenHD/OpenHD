//
// Created by consti10 on 10.12.22.
//

#include "wb_link_helper.h"

#include "wifi_command_helper.hpp"

bool openhd::wb::disable_all_frequency_checks() {
  static constexpr auto FIlE_DISABLE_ALL_FREQUENCY_CHECKS="/boot/openhd/disable_all_frequency_checks.txt";
  return OHDFilesystemUtil::exists(FIlE_DISABLE_ALL_FREQUENCY_CHECKS);
}

void openhd::wb::fixup_unsupported_settings(
    openhd::WBStreamsSettingsHolder& settings,
    std::vector<std::shared_ptr<WifiCardHolder>> m_broadcast_cards,
    std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console) {
    m_console=openhd::log::get_default();
  }
  // For now, we only check whatever the first card can do and assume the rest can do the same
  const auto first_card= m_broadcast_cards.at(0)->_wifi_card;

  const auto channel_opt= channel_from_frequency(settings.get_settings().wb_frequency);
  if(!channel_opt){
    m_console->warn("Not a vlalid frequency {}",settings.get_settings().wb_frequency);
    if(first_card.supports_5ghz){
      settings.unsafe_get_settings().wb_frequency=DEFAULT_5GHZ_FREQUENCY;
      settings.persist();
    }else{
      settings.unsafe_get_settings().wb_frequency=DEFAULT_2GHZ_FREQUENCY;
      settings.persist();
    }
  }
  // guaranteed to succeed,since we fixed a not know frequency already before
  const auto channel= channel_from_frequency(settings.get_settings().wb_frequency).value();
  if(channel.space==Space::G2_4){
    if(!first_card.supports_2ghz){
      m_console->warn("Freq {} but card doesn't support 2.4G",channel.to_string());
      settings.unsafe_get_settings().wb_frequency=DEFAULT_5GHZ_FREQUENCY;
      settings.persist();
    }
  }else{
    assert(channel.space==Space::G5_8);
    if(!first_card.supports_5ghz){
      m_console->warn("Freq {} but card doesn't support 5G",channel.to_string());
      settings.unsafe_get_settings().wb_frequency=DEFAULT_2GHZ_FREQUENCY;
      settings.persist();
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
    if(!wifi_card_supports_40Mhz_channel_width(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}

bool openhd::wb::cards_support_setting_mcs_index(
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_variable_mcs(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}


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

bool openhd::wb::set_frequency_and_channel_width_for_all_cards(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<std::shared_ptr<WifiCardHolder>>& broadcast_cards) {
  bool ret=true;
  const bool width_40= channel_width==40;
  for(const auto& card: broadcast_cards){
    if(!WifiCardCommandHelper::set_frequency_and_channel_width(card->_wifi_card,frequency,width_40)){
      return false;
    }
  }
  return ret;
}
