//
// Created by consti10 on 10.12.22.
//

#include "wb_settings_helper.h"


void openhd::wb::settings::fixup_unsupported_settings(
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

bool openhd::wb::settings::cards_support_setting_channel_width(
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_variable_mcs(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}

bool openhd::wb::settings::cards_support_setting_mcs_index(
    const std::vector<std::shared_ptr<WifiCardHolder>>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_40Mhz_channel_width(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}
