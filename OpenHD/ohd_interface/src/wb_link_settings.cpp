/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "wb_link_settings.h"

#include "include_json.hpp"

namespace openhd {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    WBLinkSettings, wb_frequency, wb_air_tx_channel_width,
    wb_gnd_rx_channel_width, wb_air_mcs_index, wb_gnd_uplink_mcs_index,
    wb_enable_stbc, wb_enable_ldpc, wb_enable_short_guard,
    wb_tx_power_milli_watt, wb_tx_power_milli_watt_armed, wb_tx_power_level,
    wb_rtl8812au_tx_pwr_idx_override, wb_rtl8812au_tx_pwr_idx_override_armed,
    wb_tx_power_mw_per_card, wb_tx_power_mw_armed_per_card,
    wb_tx_power_idx_per_card, wb_tx_power_idx_armed_per_card,
    wb_video_fec_percentage, wb_video_rate_for_mcs_adjustment_percent,
    wb_max_fec_block_size, wb_enable_rc_openhd_control,
    wb_mcs_index_via_rc_channel, wb_bw_via_rc_channel,
    wb_tx_mode_via_rc_channel,
    enable_wb_video_variable_bitrate, wb_enable_listen_only_mode, wb_pit_mode,
    wb_dev_air_set_high_retransmit_count, wb_enable_redundant_tx,
    wb_enable_retransmission, wb_enable_retransmission_video,
    wb_enable_retransmission_telemetry, wb_enable_retransmission_rc,
    wb_retransmission_history_video_ms, wb_retransmission_history_telemetry_ms,
    wb_retransmission_history_rc_ms, wb_retransmission_request_retries);

std::optional<WBLinkSettings> openhd::WBLinkSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  try {
    const auto parsed = nlohmann::json::parse(file_as_string);
    auto settings = create_default_wb_stream_settings(m_cards);
    settings.wb_frequency = parsed.value("wb_frequency", settings.wb_frequency);
    settings.wb_air_tx_channel_width = parsed.value(
        "wb_air_tx_channel_width", settings.wb_air_tx_channel_width);
    settings.wb_gnd_rx_channel_width = parsed.value(
        "wb_gnd_rx_channel_width", settings.wb_gnd_rx_channel_width);
    settings.wb_air_mcs_index =
        parsed.value("wb_air_mcs_index", settings.wb_air_mcs_index);
    settings.wb_gnd_uplink_mcs_index = parsed.value(
        "wb_gnd_uplink_mcs_index", settings.wb_gnd_uplink_mcs_index);
    settings.wb_enable_stbc =
        parsed.value("wb_enable_stbc", settings.wb_enable_stbc);
    settings.wb_enable_ldpc =
        parsed.value("wb_enable_ldpc", settings.wb_enable_ldpc);
    settings.wb_enable_short_guard =
        parsed.value("wb_enable_short_guard", settings.wb_enable_short_guard);
    settings.wb_tx_power_milli_watt =
        parsed.value("wb_tx_power_milli_watt", settings.wb_tx_power_milli_watt);
    settings.wb_tx_power_milli_watt_armed = parsed.value(
        "wb_tx_power_milli_watt_armed", settings.wb_tx_power_milli_watt_armed);
    settings.wb_tx_power_level =
        parsed.value("wb_tx_power_level", settings.wb_tx_power_level);
    settings.wb_rtl8812au_tx_pwr_idx_override =
        parsed.value("wb_rtl8812au_tx_pwr_idx_override",
                     settings.wb_rtl8812au_tx_pwr_idx_override);
    settings.wb_rtl8812au_tx_pwr_idx_override_armed =
        parsed.value("wb_rtl8812au_tx_pwr_idx_override_armed",
                     settings.wb_rtl8812au_tx_pwr_idx_override_armed);
    settings.wb_tx_power_mw_per_card = parsed.value(
        "wb_tx_power_mw_per_card", settings.wb_tx_power_mw_per_card);
    settings.wb_tx_power_mw_armed_per_card =
        parsed.value("wb_tx_power_mw_armed_per_card",
                     settings.wb_tx_power_mw_armed_per_card);
    settings.wb_tx_power_idx_per_card = parsed.value(
        "wb_tx_power_idx_per_card", settings.wb_tx_power_idx_per_card);
    settings.wb_tx_power_idx_armed_per_card =
        parsed.value("wb_tx_power_idx_armed_per_card",
                     settings.wb_tx_power_idx_armed_per_card);
    settings.wb_video_fec_percentage = parsed.value(
        "wb_video_fec_percentage", settings.wb_video_fec_percentage);
    settings.wb_video_rate_for_mcs_adjustment_percent =
        parsed.value("wb_video_rate_for_mcs_adjustment_percent",
                     settings.wb_video_rate_for_mcs_adjustment_percent);
    settings.wb_max_fec_block_size =
        parsed.value("wb_max_fec_block_size", settings.wb_max_fec_block_size);
    settings.wb_enable_rc_openhd_control =
        parsed.value("wb_enable_rc_openhd_control",
                     settings.wb_enable_rc_openhd_control);
    settings.wb_mcs_index_via_rc_channel = parsed.value(
        "wb_mcs_index_via_rc_channel", settings.wb_mcs_index_via_rc_channel);
    settings.wb_bw_via_rc_channel =
        parsed.value("wb_bw_via_rc_channel", settings.wb_bw_via_rc_channel);
    settings.wb_tx_mode_via_rc_channel = parsed.value(
        "wb_tx_mode_via_rc_channel", settings.wb_tx_mode_via_rc_channel);
    settings.enable_wb_video_variable_bitrate =
        parsed.value("enable_wb_video_variable_bitrate",
                     settings.enable_wb_video_variable_bitrate);
    settings.wb_enable_listen_only_mode = parsed.value(
        "wb_enable_listen_only_mode", settings.wb_enable_listen_only_mode);
    settings.wb_pit_mode = parsed.value("wb_pit_mode", settings.wb_pit_mode);
    settings.wb_dev_air_set_high_retransmit_count =
        parsed.value("wb_dev_air_set_high_retransmit_count",
                     settings.wb_dev_air_set_high_retransmit_count);
    settings.wb_enable_redundant_tx =
        parsed.value("wb_enable_redundant_tx", settings.wb_enable_redundant_tx);
    settings.wb_enable_retransmission = parsed.value(
        "wb_enable_retransmission", settings.wb_enable_retransmission);
    settings.wb_enable_retransmission_video =
        parsed.value("wb_enable_retransmission_video",
                     settings.wb_enable_retransmission_video);
    settings.wb_enable_retransmission_telemetry =
        parsed.value("wb_enable_retransmission_telemetry",
                     settings.wb_enable_retransmission_telemetry);
    settings.wb_enable_retransmission_rc = parsed.value(
        "wb_enable_retransmission_rc", settings.wb_enable_retransmission_rc);
    settings.wb_retransmission_history_video_ms =
        parsed.value("wb_retransmission_history_video_ms",
                     settings.wb_retransmission_history_video_ms);
    settings.wb_retransmission_history_telemetry_ms =
        parsed.value("wb_retransmission_history_telemetry_ms",
                     settings.wb_retransmission_history_telemetry_ms);
    settings.wb_retransmission_history_rc_ms =
        parsed.value("wb_retransmission_history_rc_ms",
                     settings.wb_retransmission_history_rc_ms);
    settings.wb_retransmission_request_retries =
        parsed.value("wb_retransmission_request_retries",
                     settings.wb_retransmission_request_retries);

    // Migration: If we loaded a legacy config, the vectors might be empty.
    // Populate them from the legacy single values (which are also loaded).
    if (settings.wb_tx_power_mw_per_card.empty()) {
      for (int i = 0; i < MAX_WIFI_CARDS; i++) {
        settings.wb_tx_power_mw_per_card.push_back(
            settings.wb_tx_power_milli_watt);
        settings.wb_tx_power_mw_armed_per_card.push_back(
            settings.wb_tx_power_milli_watt_armed);
        settings.wb_tx_power_idx_per_card.push_back(
            settings.wb_rtl8812au_tx_pwr_idx_override);
        settings.wb_tx_power_idx_armed_per_card.push_back(
            settings.wb_rtl8812au_tx_pwr_idx_override_armed);
      }
    }
    return settings;
  } catch (const nlohmann::json::exception &ex) {
    std::stringstream ss;
    ss << "openhd_json_parse error:" << ex.what() << "\n";
    ss << file_as_string;
    std::cout << ss.str() << std::endl;
  }
  return std::nullopt;
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
  settings.wb_gnd_rx_channel_width = DEFAULT_GND_RX_CHANNEL_WIDTH;
  // custom hardware only has one wifi card
  if (wifibroadcast_cards.at(0).is_openhd_rtl8812au_x20()) {
    // Already a lot lol
    settings.wb_rtl8812au_tx_pwr_idx_override = 10;
  } else {
    // Should work even on ali cards without burning them
    settings.wb_rtl8812au_tx_pwr_idx_override = 10;
  }
  if (OHDPlatform::instance().is_x20()) {
    settings.wb_enable_stbc = true;
    settings.wb_enable_ldpc = true;
  }
  if (wifibroadcast_cards.at(0).is_rtl88x2eu()) {
    settings.wb_enable_stbc = true;
    settings.wb_enable_ldpc = true;
    // There are no single Antenna 88x2eu cards
  }
  for (int i = 0; i < MAX_WIFI_CARDS; i++) {
    settings.wb_tx_power_mw_per_card.push_back(settings.wb_tx_power_milli_watt);
    settings.wb_tx_power_mw_armed_per_card.push_back(
        settings.wb_tx_power_milli_watt_armed);
    settings.wb_tx_power_idx_per_card.push_back(
        settings.wb_rtl8812au_tx_pwr_idx_override);
    settings.wb_tx_power_idx_armed_per_card.push_back(
        settings.wb_rtl8812au_tx_pwr_idx_override_armed);
  }
  return settings;
}
}  // namespace openhd
