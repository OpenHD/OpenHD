//
// Created by consti10 on 19.09.23.
//
#include "wb_link_settings.h"

#include "include_json.hpp"

namespace openhd {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    WBLinkSettings, wb_frequency, wb_air_tx_channel_width, wb_air_mcs_index,
    wb_enable_stbc, wb_enable_ldpc, wb_enable_short_guard,
    wb_tx_power_milli_watt, wb_tx_power_milli_watt_armed,
    wb_rtl8812au_tx_pwr_idx_override, wb_rtl8812au_tx_pwr_idx_override_armed,
    wb_video_fec_percentage, wb_video_rate_for_mcs_adjustment_percent,
    wb_max_fec_block_size, wb_mcs_index_via_rc_channel, wb_bw_via_rc_channel,
    enable_wb_video_variable_bitrate, wb_enable_listen_only_mode,
    wb_dev_air_set_high_retransmit_count);

std::optional<WBLinkSettings> openhd::WBLinkSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<WBLinkSettings>(file_as_string);
}

std::string WBLinkSettingsHolder::imp_serialize(
    const openhd::WBLinkSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

WBLinkSettings create_default_wb_stream_settings(
    const std::vector<WiFiCard> &wifibroadcast_cards) {
  assert(!wifibroadcast_cards.empty());
  const auto &first_card = wifibroadcast_cards.at(0);
  assert(first_card.supports_5GHz() || first_card.supports_2GHz());
  const bool use_5ghz = wifibroadcast_cards.at(0).supports_5GHz();
  WBLinkSettings settings{};
  if (use_5ghz) {
    settings.wb_frequency = DEFAULT_5GHZ_FREQUENCY;
  } else {
    settings.wb_frequency = DEFAULT_2GHZ_FREQUENCY;
  }
  // custom hardware only has one wifi card
  if (wifibroadcast_cards.at(0).is_openhd_rtl8812au_x20()) {
    // Already a lot lol
    settings.wb_rtl8812au_tx_pwr_idx_override = 1;
  } else {
    // Should work even on ali cards without burning them
    settings.wb_rtl8812au_tx_pwr_idx_override = 10;
  }
  if (OHDPlatform::instance().is_x20()) {
    settings.wb_enable_stbc = true;
    settings.wb_enable_ldpc = true;
  }
  return settings;
}
}  // namespace openhd
