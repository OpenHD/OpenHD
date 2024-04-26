//
// Created by consti10 on 10.12.22.
//

#include "wb_link_helper.h"

#include "wb_link_rate_helper.hpp"
#include "wifi_command_helper.h"
// #include "wifi_command_helper2.h"
#include "openhd_spdlog_include.h"

bool openhd::wb::disable_all_frequency_checks() {
  static constexpr auto FIlE_DISABLE_ALL_FREQUENCY_CHECKS =
      "/boot/openhd/disable_all_frequency_checks.txt";
  return OHDFilesystemUtil::exists(FIlE_DISABLE_ALL_FREQUENCY_CHECKS);
}

bool openhd::wb::all_cards_support_frequency(
    uint32_t frequency, const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console = nullptr) {
  // and check if all cards support the frequency
  for (const auto& card : m_broadcast_cards) {
    if (!wifi_card_supports_frequency(card, frequency)) {
      if (m_console) {
        m_console->debug("Card {} doesn't support frequency {}",
                         card.device_name, frequency);
      }
      return false;
    }
  }
  return true;
}

bool openhd::wb::all_cards_support_frequency_and_channel_width(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console) {
  for (const auto& card : m_broadcast_cards) {
    if (!wifi_card_supports_frequency_channel_width(card, frequency,
                                                    channel_width)) {
      if (m_console) {
        m_console->debug(
            "Card {} doesn't support frequency/channel width {}:{}",
            card.device_name, frequency, channel_width);
      }
      return false;
    }
  }
  return true;
}

bool openhd::wb::any_card_support_frequency(
    uint32_t frequency, const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console) {
  bool any_supports_frequency = false;
  for (const auto& card : m_broadcast_cards) {
    if (wifi_card_supports_frequency(card, frequency)) {
      any_supports_frequency = true;
    }
  }
  return any_supports_frequency;
}

bool openhd::wb::set_frequency_and_channel_width_for_all_cards(
    uint32_t frequency, uint32_t channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards) {
  bool ret = true;
  for (const auto& card : m_broadcast_cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      break;
    }
    if (card.type == WiFiCardType::OPENHD_RTL_88X2AU ||
        card.type == WiFiCardType::OPENHD_RTL_88X2BU ||
        card.type == WiFiCardType::OPENHD_RTL_8852BU) {
      const int type = card.type == WiFiCardType::OPENHD_RTL_88X2AU ? 0 : 1;
      wifi::commandhelper::openhd_driver_set_frequency_and_channel_width(
          type, card.device_name, frequency, channel_width);
    } else {
      const bool success =
          wifi::commandhelper::iw_set_frequency_and_channel_width(
              card.device_name, frequency, channel_width);
      // const bool
      // success=wifi::commandhelper2::set_wifi_frequency_and_log_result(card->get_wifi_card().interface_name,frequency,channel_width);
      if (!success) {
        ret = false;
      }
    }
  }
  return ret;
}

void openhd::wb::set_tx_power_for_all_cards(
    int tx_power_mw, int rtl8812au_tx_power_index_override,
    const std::vector<WiFiCard>& m_broadcast_cards) {
  for (const auto& card : m_broadcast_cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      break;
    }
    if (card.type == WiFiCardType::OPENHD_RTL_88X2AU) {
      openhd::log::get_default()->debug("RTL8812AU tx_pwr_idx_override: {}",
                                        rtl8812au_tx_power_index_override);
      wifi::commandhelper::iw_set_tx_power(card.device_name,
                                           rtl8812au_tx_power_index_override);
    } else {
      const auto tx_power_mbm = openhd::milli_watt_to_mBm(tx_power_mw);
      openhd::log::get_default()->debug("Tx power mW:{} mBm:{}", tx_power_mw,
                                        tx_power_mbm);
      if (card.type == WiFiCardType::OPENHD_RTL_88X2BU) {
        wifi::commandhelper::openhd_driver_set_tx_power(card.device_name,
                                                        tx_power_mbm);
      } else if (card.type == WiFiCardType::OPENHD_RTL_8852BU) {
        wifi::commandhelper::openhd_driver_set_tx_power(card.device_name,
                                                        tx_power_mbm);
      } else {
        wifi::commandhelper::iw_set_tx_power(card.device_name, tx_power_mbm);
      }
    }
  }
}

std::vector<std::string> openhd::wb::get_card_names(
    const std::vector<WiFiCard>& cards) {
  std::vector<std::string> ret{};
  for (const auto& card : cards) {
    ret.push_back(card.device_name);
  }
  return ret;
}

bool openhd::wb::any_card_supports_stbc_ldpc_sgi(
    const std::vector<WiFiCard>& cards) {
  for (const auto& card : cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      return true;
    }
    if (card.type == WiFiCardType::OPENHD_RTL_88X2AU ||
        card.type == WiFiCardType::OPENHD_RTL_88X2BU ||
        card.type == WiFiCardType::OPENHD_RTL_8852BU) {
      return true;
    }
  }
  return false;
}

std::vector<openhd::WifiChannel> openhd::wb::get_scan_channels_frequencies(
    const WiFiCard& card, int channels_to_scan) {
  std::vector<openhd::WifiChannel> ret;
  if (channels_to_scan == 0) {
    // OpenHD channels 1 to 5 only
    return openhd::get_openhd_channels_1_to_5();
  }
  if (channels_to_scan == 1) {
    auto supported = card.supported_frequencies_2G;
    return openhd::frequencies_to_channels(supported);
  }
  auto supported = card.supported_frequencies_5G;
  return openhd::frequencies_to_channels(supported);
}

std::vector<openhd::WifiChannel> openhd::wb::get_analyze_channels_frequencies(
    const WiFiCard& card, int channels_to_scan) {
  std::vector<openhd::WifiChannel> ret;
  if (channels_to_scan == 0) {
    // OpenHD channels 1 to 5 only
    return openhd::get_openhd_channels_1_to_5();
  }
  if (channels_to_scan == 1) {
    // 2.4G but we scan in 40Mhz increments
    auto supported = card.supported_frequencies_2G;
    return openhd::frequencies_to_channels(supported);
  }
  return openhd::filter_ht40plus_only(card.supported_frequencies_5G);
}

bool openhd::wb::has_any_rtl8812au(const std::vector<WiFiCard>& cards) {
  for (const auto& card : cards) {
    if (card.type == WiFiCardType::OPENHD_RTL_88X2AU) {
      return true;
    }
  }
  return false;
}

bool openhd::wb::has_any_non_rtl8812au(const std::vector<WiFiCard>& cards) {
  for (const auto& card : cards) {
    if (card.type != WiFiCardType::OPENHD_RTL_88X2AU) {
      return true;
    }
  }
  return false;
}

void openhd::wb::takeover_cards_monitor_mode(
    const std::vector<WiFiCard>& cards,
    std::shared_ptr<spdlog::logger> console) {
  console->debug("takeover_cards_monitor_mode() begin");
  // We need to take "ownership" from the system over the cards used for monitor
  // mode / wifibroadcast. This can be different depending on the OS we are
  // running on - in general, we try to go for the following with openhd: Have
  // network manager running on the host OS - the nice thing about network
  // manager is that we can just tell it to ignore the cards we are doing
  // wifibroadcast with, instead of killing all processes that might interfere
  // with wifibroadcast and therefore making other networking incredibly hard.
  // Tell network manager to ignore the cards we want to do wifibroadcast on
  bool emulate = false;
  for (const auto& card : cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      return;
    }
    wifi::commandhelper::nmcli_set_device_managed_status(card.device_name,
                                                         false);
  }
  if (!emulate) {
    wifi::commandhelper::rfkill_unblock_all();
    // Apparently, we need to give nm / whoever a bit time before we start
    // putting the cards into monitor mode not pretty, but works.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // now we can enable monitor mode on the given cards.
    for (const auto& card : cards) {
      wifi::commandhelper::ip_link_set_card_state(card.device_name, false);
      wifi::commandhelper::iw_enable_monitor_mode(card.device_name);
      wifi::commandhelper::ip_link_set_card_state(card.device_name, true);
      // wifi::commandhelper2::set_wifi_monitor_mode(card->_wifi_card.interface_name);
    }
  }
  console->debug("takeover_cards_monitor_mode() end");
}

void openhd::wb::giveback_cards_monitor_mode(
    const std::vector<WiFiCard>& cards,
    std::shared_ptr<spdlog::logger> console) {
  for (const auto& card : cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      return;
    }
    wifi::commandhelper::nmcli_set_device_managed_status(card.device_name,
                                                         true);
  }
}

bool openhd::wb::validate_frequency_change(
    int new_frequency, int current_channel_width,
    const std::vector<WiFiCard>& m_broadcast_cards,
    const std::shared_ptr<spdlog::logger>& m_console) {
  if (openhd::wb::disable_all_frequency_checks()) {
    m_console->warn("Not sanity checking frequency");
    return true;
  }
  if (new_frequency == 2484 && current_channel_width == 40) {
    m_console->warn("40Mhz not supported on 2484Mhz");
    return false;
  }
  if (!openhd::wb::all_cards_support_frequency(new_frequency, m_broadcast_cards,
                                               m_console)) {
    m_console->warn(
        "Cannot change frequency, at least one card doesn't support");
    return false;
  }
  if (!openhd::wb::all_cards_support_frequency_and_channel_width(
          new_frequency, current_channel_width, m_broadcast_cards, m_console)) {
    m_console->warn(
        "Cannot change frequency, 40Mhz not allowed (on at least one card)");
    return false;
  }
  return true;
}

bool openhd::wb::validate_air_channel_width_change(
    int new_channel_width, const WiFiCard& card,
    const std::shared_ptr<spdlog::logger>& m_console) {
  if (!openhd::is_valid_channel_width(new_channel_width)) {
    m_console->warn("Invalid channel width {}", new_channel_width);
    return false;
  }
  // We only have one tx card, check if it supports injecting with 40Mhz channel
  // width:
  if (new_channel_width == 40 &&
      !wifi_card_supports_40Mhz_channel_width_injection(card)) {
    m_console->warn("Cannot change channel width, not supported by card");
    return false;
  }
  return true;
}

bool openhd::wb::validate_air_mcs_index_change(
    int new_mcs_index, const WiFiCard& card,
    const std::shared_ptr<spdlog::logger>& m_console) {
  if (!openhd::is_valid_mcs_index(new_mcs_index)) {
    m_console->warn("Invalid mcs index{}", new_mcs_index);
    return false;
  }
  if (!wifi_card_supports_variable_mcs(card)) {
    m_console->warn(
        "Cannot change mcs index, card doesn't support variable MCS");
    return false;
  }
  return true;
}

int openhd::wb::calculate_bitrate_for_wifi_config_kbits(
    const WiFiCard& card, int frequency_mhz, int channel_width_mhz,
    int mcs_index, int dev_adjustment_percent, bool debug_log) {
  // First we calculate the theoretical rate for the current "wifi config" aka
  // taking mcs index, channel width, ... into account
  const auto wifi_space = openhd::get_space_from_frequency(frequency_mhz);
  const int max_rate_for_current_wifi_config_without_adjust =
      get_max_rate_possible(card, wifi_space, mcs_index,
                            channel_width_mhz == 40);
  const int max_rate_for_current_wifi_config = multiply_by_perc(
      max_rate_for_current_wifi_config_without_adjust, dev_adjustment_percent);
  if (debug_log) {
    auto m_console = openhd::log::get_default();
    m_console->debug(
        "Max rate for {}@{}Mhz MCS:{} dev_adjustment:{} is {} kBit/s",
        frequency_mhz, channel_width_mhz, mcs_index, dev_adjustment_percent,
        max_rate_for_current_wifi_config);
  }
  return max_rate_for_current_wifi_config;
}

std::optional<int> openhd::wb::RCChannelHelper::get_mcs_from_rc_channel(
    int channel_index, std::shared_ptr<spdlog::logger>& m_console) {
  const auto rc_channels_opt = get_fc_reported_rc_channels();
  if (!rc_channels_opt.has_value()) {
    // No data from the FC yet, do nothing
    // m_console->debug("No rc channels from RC, MCS via RC unavailable");
    return std::nullopt;
  }
  const auto& rc_channels = rc_channels_opt.value();
  // check if we are in bounds of array (better be safe than sorry, in case user
  // manually messes up a number)
  if (!(channel_index >= 0 && channel_index < rc_channels.size())) {
    m_console->debug("Invalid channel index {}", channel_index);
    return std::nullopt;
  }
  const auto mcs_channel_value_pwm = rc_channels[channel_index];
  // UINT16_MAX means ignore channel
  if (mcs_channel_value_pwm == UINT16_MAX) {
    m_console->debug("Disabled channel {}: {}", channel_index,
                     mcs_channel_value_pwm);
    return std::nullopt;
  }
  // mavlink says pwm in [1000, 2000] range - but from my testing with frsky for
  // example, it is quite common for a switch (for example) to be at for example
  // [988 - 2012] us which is why we accept a [900 ... 2100] range here
  if (mcs_channel_value_pwm < 900 || mcs_channel_value_pwm > 2100) {
    m_console->debug("Invalid channel data on channel {}: {}", channel_index,
                     mcs_channel_value_pwm);
    // most likely invalid data, discard
    return std::nullopt;
  }
  // We simply pre-define a range (pwm: [900,...,2100]
  // [900 ... 1200] : MCS0
  // [1200 ... 1400] : MCS1
  // [1400 ... 1600] : MCS2
  // [1600 ... 1800] : MCS3
  // [1800 ... 2100] : MCS 4
  int mcs_index = 0;
  if (mcs_channel_value_pwm > 1800) {
    mcs_index = 4;
  } else if (mcs_channel_value_pwm > 1600) {
    mcs_index = 3;
  } else if (mcs_channel_value_pwm > 1400) {
    mcs_index = 2;
  } else if (mcs_channel_value_pwm > 1200) {
    mcs_index = 1;
  }
  return mcs_index;
}

std::optional<std::array<int, 18>>
openhd::wb::RCChannelHelper::get_fc_reported_rc_channels() {
  std::lock_guard<std::mutex> guard(m_rc_channels_mutex);
  return m_rc_channels;
}

void openhd::wb::RCChannelHelper::set_rc_channels(
    const std::array<int, 18>& rc_channels) {
  std::lock_guard<std::mutex> guard(m_rc_channels_mutex);
  m_rc_channels = rc_channels;
}

std::optional<uint8_t> openhd::wb::RCChannelHelper::get_bw_from_rc_channel(
    int channel_index) {
  if (channel_index < 0 || channel_index >= 18) return std::nullopt;
  const auto rc_channels_opt = get_fc_reported_rc_channels();
  if (!rc_channels_opt.has_value()) {
    // No data from the FC yet, do nothing
    return std::nullopt;
  }
  const auto bw_channel_value_pwm = rc_channels_opt.value()[channel_index];
  if (bw_channel_value_pwm == UINT16_MAX) {
    return std::nullopt;
  }
  // mavlink says pwm in [1000, 2000] range - but from my testing with frsky for
  // example, it is quite common for a switch (for example) to be at for example
  // [988 - 2012] us which is why we accept a [900 ... 2100] range here
  if (bw_channel_value_pwm < 900 || bw_channel_value_pwm > 2100) {
    // most likely invalid data, discard
    return std::nullopt;
  }
  if (bw_channel_value_pwm > 1500) return 40;
  return 20;
}
