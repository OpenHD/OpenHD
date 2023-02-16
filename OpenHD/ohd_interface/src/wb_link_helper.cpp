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

bool openhd::wb::cards_support_setting_channel_width(
    const std::vector<WiFiCard>& m_broadcast_cards) {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_40Mhz_channel_width(card_handle)){
      return false;
    }
  }
  return true;
}

bool openhd::wb::cards_support_setting_mcs_index(
    const std::vector<WiFiCard>& m_broadcast_cards) {
  for(const auto& card: m_broadcast_cards){
    if(!wifi_card_supports_variable_mcs(card)){
      return false;
    }
  }
  return true;
}


bool openhd::wb::cards_support_frequency(
    uint32_t frequency,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const OHDPlatform& platform,
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

void openhd::wb::fixup_unsupported_settings(
    openhd::WBStreamsSettingsHolder& settings,
    const std::vector<WiFiCard>& m_broadcast_cards,
    std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console) {
    m_console=openhd::log::get_default();
  }
  // For now, we only check whatever the first card can do and assume the rest can do the same
  const WiFiCard first_card= m_broadcast_cards.at(0);

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
  if(!cards_support_setting_mcs_index(m_broadcast_cards)){
    m_console->warn("Card {} doesn't support setting MCS index, applying defaults",first_card.device_name);
    // cards that do not support changing the mcs index are always fixed to mcs3 on openhd drivers
    settings.unsafe_get_settings().wb_mcs_index=3;
    settings.persist();
  }
  settings.persist();
}

bool openhd::wb::set_frequency_and_channel_width_for_all_cards(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards) {
  bool ret=true;
  for(const auto& card: m_broadcast_cards){
    const bool success=wifi::commandhelper::iw_set_frequency_and_channel_width(card.device_name,frequency,channel_width);
    //const bool success=wifi::commandhelper2::set_wifi_frequency_and_log_result(card->get_wifi_card().interface_name,frequency,channel_width);
    if(!success){
      ret=false;
    }
  }
  return ret;
}

uint32_t openhd::wb::rtl8812au_get_measured_max_rate(uint32_t mcs_index) {
  switch (mcs_index) {
    case 0:
      return 5000;
    case 1:
      return 8000;
    case 2:
      return 12000;
    case 3:
      return 15000;
    case 4:
      return 20000;
    case 5:
      return 30000; // not tested, since r.n we do not have encoders for such bitrates anyways
    case 6:
      return 40000; // not tested, since r.n we do not have encoders for such bitrates anyways
    default:
      break;
  }
  openhd::log::get_default()->warn("Invalid mcs index");
  assert(true);
  return 5000;
}

std::vector<std::string> openhd::wb::get_card_names(const std::vector<WiFiCard>& cards) {
  std::vector<std::string> ret{};
  for(const auto& card: cards){
    ret.push_back(card.device_name);
  }
  return ret;
}

bool openhd::wb::has_any_rtl8812au(const std::vector<WiFiCard>& cards) {
  for(const auto& card: cards){
    if(card.type==WiFiCardType::Realtek8812au){
      return true;
    }
  }
  return false;
}
