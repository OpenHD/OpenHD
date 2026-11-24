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

#include <algorithm>
#include <array>

#include "include_json.hpp"

namespace openhd {

namespace {

constexpr std::array rc_profile_keys{
    "wb_mcs_index_via_rc_channel", "wb_bw_via_rc_channel",
    "wb_mcs_rc_ranges", "wb_bw_rc_ranges"};

nlohmann::json read_wb_settings_json(const std::string& path) {
  const auto saved = OHDFilesystemUtil::opt_read_file(path, false);
  if (!saved) return nlohmann::json::object();
  auto parsed = nlohmann::json::parse(*saved, nullptr, false);
  return parsed.is_object() ? parsed : nlohmann::json::object();
}

int migrate_tx_power_level(const int value) {
  // Migrate the short-lived four-level implementation and its disabled
  // default to the new mandatory five-target control.
  switch (value) {
    case -1:
    case 0:
      // 0 is now the bottom of the normalized 0..150 control. Old files that
      // used enum 0 also selected the lowest rung, so retaining 0 is safe.
      return WB_TX_POWER_LEVEL_MIN;
    case 1:
      return WB_TX_POWER_LEVEL_40;
    case 2:
      return WB_TX_POWER_LEVEL_80;
    case 3:
      return WB_TX_POWER_LEVEL_100;
    default:
      return value;
  }
}

}  // namespace

WBLinkSettingsHolder::WBLinkSettingsHolder(
    OHDProfile profile, std::vector<WiFiCard> wifibroadcast_cards,
    std::string settings_directory)
    : PersistentSettings<WBLinkSettings>(settings_directory),
      m_profile(std::move(profile)),
      m_cards(std::move(wifibroadcast_cards)),
      m_settings_directory(std::move(settings_directory)) {
  if (!m_cards.empty()) {
    const auto saved = read_wb_settings_json(
        m_settings_directory + get_unique_filename());
    m_had_staged_rc_profiles = !saved.empty() &&
                               saved.size() <= rc_profile_keys.size();
    if (m_had_staged_rc_profiles) {
      for (const auto& entry : saved.items()) {
        if (std::find(rc_profile_keys.begin(), rc_profile_keys.end(), entry.key()) ==
            rc_profile_keys.end()) {
          m_had_staged_rc_profiles = false;
          break;
        }
      }
    }
  }
  init(!m_cards.empty());
  if (m_cards.empty()) return;

  // Keep hardware-specific defaults and normal first-start provisioning when
  // the only saved settings were configured before a radio was available.
  if (m_had_staged_rc_profiles) _created_defaults = true;
  auto& settings = unsafe_get_settings();
  bool changed = m_had_staged_rc_profiles;
  for (auto* width : {&settings.wb_air_tx_channel_width,
                      &settings.wb_gnd_rx_channel_width}) {
    if (*width == 5) {
      *width = 20;
      changed = true;
    }
  }
  if (changed) persist();
}

std::vector<Setting> WBLinkSettingsHolder::get_rc_profile_settings(
    std::function<void()> on_bandwidth_changed) {
  std::vector<Setting> result;
  const auto add_channel = [&](const char* id, auto member, bool bandwidth) {
    result.push_back(Setting{id, IntSetting{get_rc_channel(member),
        [this, member, bandwidth, on_bandwidth_changed](std::string, int value) {
          if (value < 0 || value > 18) return false;
          {
            std::lock_guard<std::mutex> lock(m_rc_profiles_mutex);
            auto& channel = unsafe_get_settings().*member;
            if (static_cast<int>(channel) == value) return true;
            channel = value;
            PersistentSettings::persist();
          }
          if (bandwidth && on_bandwidth_changed) on_bandwidth_changed();
          return true;
        }}});
  };
  const auto add_profile = [&](const char* id, rc::RangeKind kind,
                               std::string WBLinkSettings::*member) {
    result.push_back(Setting{id, StringSetting{get_rc_ranges(member),
        [this, kind, member, on_bandwidth_changed](std::string, std::string value) {
          if (!rc::parse_ranges(value, kind)) return false;
          if (set_rc_ranges(member, std::move(value)) &&
              kind == rc::RangeKind::Bandwidth && on_bandwidth_changed)
            on_bandwidth_changed();
          return true;
        }}});
  };
  add_channel(WB_MCS_INDEX_VIA_RC_CHANNEL, &WBLinkSettings::wb_mcs_index_via_rc_channel, false);
  add_channel(WB_BW_VIA_RC_CHANNEL, &WBLinkSettings::wb_bw_via_rc_channel, true);
  add_profile("RC_MCS_RANGES", rc::RangeKind::Mcs, &WBLinkSettings::wb_mcs_rc_ranges);
  add_profile("RC_BW_RANGES", rc::RangeKind::Bandwidth, &WBLinkSettings::wb_bw_rc_ranges);
  for (auto& setting : result) {
    setting.seed_allowed = created_defaults() && !m_had_staged_rc_profiles &&
                           !m_cards.empty();
  }
  return result;
}

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
    wb_rc_settings_base_channel,
    wb_mcs_index_via_rc_channel, wb_bw_via_rc_channel,
    wb_bw_rc_ranges, wb_mcs_rc_ranges, wb_tx_mode_rc_ranges, wb_fhss_rc_ranges,
    wb_tx_mode_via_rc_channel, wb_fhss_via_rc_channel,
    enable_wb_video_variable_bitrate, wb_enable_listen_only_mode, wb_pit_mode,
    wb_dev_air_set_high_retransmit_count, wb_enable_redundant_tx,
    wb_enable_adaptive_channel, wb_enable_esp_analyse, wb_enable_fhss, wb_fhss_slot_ms,
    wb_enable_retransmission, wb_enable_retransmission_video,
    wb_enable_retransmission_telemetry, wb_enable_retransmission_rc,
    wb_retransmission_history_video_ms, wb_retransmission_history_telemetry_ms,
    wb_retransmission_history_rc_ms, wb_retransmission_request_retries,
    wb_udp_data_enabled, wb_udp_data_input_port, wb_udp_data_output_port,
    wb_udp_data_fec_percentage, wb_udp_data_link_budget_percent,
    wb_udp_data_max_bitrate_kbits, wb_enable_devourer_logging);

std::optional<WBLinkSettings> openhd::WBLinkSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  try {
    const auto parsed = nlohmann::json::parse(file_as_string);
    auto settings = create_default();
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
    settings.wb_tx_power_level = migrate_tx_power_level(
        parsed.value("wb_tx_power_level", settings.wb_tx_power_level));
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
    settings.wb_rc_settings_base_channel = parsed.value(
        "wb_rc_settings_base_channel", settings.wb_rc_settings_base_channel);
    settings.wb_mcs_index_via_rc_channel = parsed.value(
        "wb_mcs_index_via_rc_channel", settings.wb_mcs_index_via_rc_channel);
    settings.wb_bw_via_rc_channel =
        parsed.value("wb_bw_via_rc_channel", settings.wb_bw_via_rc_channel);
    settings.wb_bw_rc_ranges = parsed.value("wb_bw_rc_ranges", std::string(
        rc::initial_ranges(rc::RangeKind::Bandwidth, settings.wb_bw_via_rc_channel > 0)));
    settings.wb_mcs_rc_ranges = parsed.value("wb_mcs_rc_ranges", std::string(
        rc::initial_ranges(rc::RangeKind::Mcs, settings.wb_mcs_index_via_rc_channel > 0)));
    if (!rc::parse_ranges(settings.wb_bw_rc_ranges, rc::RangeKind::Bandwidth))
      settings.wb_bw_rc_ranges = rc::initial_ranges(rc::RangeKind::Bandwidth, settings.wb_bw_via_rc_channel > 0);
    if (!rc::parse_ranges(settings.wb_mcs_rc_ranges, rc::RangeKind::Mcs))
      settings.wb_mcs_rc_ranges = rc::initial_ranges(rc::RangeKind::Mcs, settings.wb_mcs_index_via_rc_channel > 0);
    settings.wb_tx_mode_via_rc_channel = parsed.value(
        "wb_tx_mode_via_rc_channel", settings.wb_tx_mode_via_rc_channel);
    settings.wb_fhss_via_rc_channel = parsed.value(
        "wb_fhss_via_rc_channel", settings.wb_fhss_via_rc_channel);
    settings.wb_tx_mode_rc_ranges = parsed.value("wb_tx_mode_rc_ranges", settings.wb_tx_mode_rc_ranges);
    settings.wb_fhss_rc_ranges = parsed.value("wb_fhss_rc_ranges", settings.wb_fhss_rc_ranges);
    if (!rc::parse_ranges(settings.wb_tx_mode_rc_ranges, rc::RangeKind::TxMode))
      settings.wb_tx_mode_rc_ranges = rc::default_tx_mode_ranges;
    if (!rc::parse_ranges(settings.wb_fhss_rc_ranges, rc::RangeKind::Fhss))
      settings.wb_fhss_rc_ranges = rc::default_fhss_ranges;
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
    settings.wb_enable_esp_analyse = parsed.value("wb_enable_esp_analyse", settings.wb_enable_esp_analyse);
    settings.wb_enable_adaptive_channel = parsed.value(
        "wb_enable_adaptive_channel", settings.wb_enable_adaptive_channel);
    settings.wb_enable_fhss =
        parsed.value("wb_enable_fhss", settings.wb_enable_fhss);
    settings.wb_fhss_slot_ms =
        parsed.value("wb_fhss_slot_ms", settings.wb_fhss_slot_ms);
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
    settings.wb_udp_data_enabled = parsed.value(
        "wb_udp_data_enabled", settings.wb_udp_data_enabled);
    settings.wb_udp_data_input_port = parsed.value(
        "wb_udp_data_input_port", settings.wb_udp_data_input_port);
    settings.wb_udp_data_output_port = parsed.value(
        "wb_udp_data_output_port", settings.wb_udp_data_output_port);
    settings.wb_udp_data_fec_percentage = parsed.value(
        "wb_udp_data_fec_percentage", settings.wb_udp_data_fec_percentage);
    settings.wb_udp_data_link_budget_percent = parsed.value(
        "wb_udp_data_link_budget_percent",
        settings.wb_udp_data_link_budget_percent);
    settings.wb_udp_data_max_bitrate_kbits = parsed.value(
        "wb_udp_data_max_bitrate_kbits",
        settings.wb_udp_data_max_bitrate_kbits);
    settings.wb_enable_devourer_logging = parsed.value(
        "wb_enable_devourer_logging", settings.wb_enable_devourer_logging);

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
  if (m_cards.empty()) {
    // Preserve previous radio configuration, or leave radio defaults unset
    // until a real card is discovered.
    auto saved = read_wb_settings_json(m_settings_directory + get_unique_filename());
    saved[rc_profile_keys[0]] = data.wb_mcs_index_via_rc_channel;
    saved[rc_profile_keys[1]] = data.wb_bw_via_rc_channel;
    saved[rc_profile_keys[2]] = data.wb_mcs_rc_ranges;
    saved[rc_profile_keys[3]] = data.wb_bw_rc_ranges;
    return saved.dump(4);
  }
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
