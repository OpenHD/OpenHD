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

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_

#include <utility>

#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"
#include "wifi_card.h"

namespace openhd {

static constexpr auto DEFAULT_5GHZ_FREQUENCY =
    5745;  // Channel 149 / OpenHD race band 2
static constexpr auto DEFAULT_2GHZ_FREQUENCY =
    2452;  // Channel 9 / is a 20Mhz channel / No openhd band in 2.4G
// highest MCS where modulation is still QPSK
static constexpr auto DEFAULT_MCS_INDEX = 2;
// We always use a MCS index of X for the uplink, since (compared to the video
// link) it requires a negligible amount of bandwidth and for those using RC
// over OpenHD, we have the benefit that the range of RC is "more" than the
// range for video
static constexpr auto WB_GND_UPLINK_MCS_INDEX = 0;
static constexpr auto DEFAULT_CHANNEL_WIDTH = 20;
static constexpr auto DEFAULT_GND_RX_CHANNEL_WIDTH = 20;
// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that
// ever made it trough though) This value seems a bit high to me, so I am going
// with a default of "1800" (which should be 18.0 dBm ) Used to be in dBm, but
// mW really is more verbose to the user - we convert from mW to dBm when using
// the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT = 25;
// by default, we do not differentiate (to not confuse the user)
static constexpr auto WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED = 0;
// User-facing power targets. These are five simple choices; the backend maps
// them to the appropriate calibrated control for each radio.
static constexpr int WB_TX_POWER_LEVEL_20 = 20;
static constexpr int WB_TX_POWER_LEVEL_40 = 40;
static constexpr int WB_TX_POWER_LEVEL_60 = 60;
static constexpr int WB_TX_POWER_LEVEL_80 = 80;
static constexpr int WB_TX_POWER_LEVEL_100 = 100;
// tx power index 22 is about 25mW on asus, but on some card(s) that can be too
// much already (especially on custom HW). therefore, this default value is
// written at run time (see below)
static constexpr auto DEFAULT_RTL8812AU_TX_POWER_INDEX = 0;
// by default, we do not differentiate (to not confuse users)
static constexpr auto RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED = 0;
// LDPC is enabled by default - drivers that don't support ldpc during rx do not
// exist anymore, and if the tx driver doesn't support it, it is just omitted.
static constexpr bool DEFAULT_ENABLE_LDPC = false;
// SHORT GUARD - doesn't really have that much of an benefit regarding bitrate,
// so we set it off by default (use long guard)
static constexpr bool DEFAULT_ENABLE_SHORT_GUARD = false;

// Set to 0 for fec auto block length
// Set to 1 or greater for fixed k fec
// Default to auto since 2.2.5-evo
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH_AUTO = 0;
static constexpr auto DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH =
    WB_VIDEO_FEC_BLOCK_LENGTH_AUTO;
// FEC can fixup packet loss, as long as is statistically well distributed (no
// big gaps) if there are many big gaps, increasing the FEC percentage often
// doesn't help, it is better to reduce the key frame interval of your camera in
// this case
static constexpr auto DEFAULT_WB_VIDEO_FEC_PERCENTAGE = 20;
// -1 = use openhd recommended for this platform
static constexpr uint32_t DEFAULT_MAX_FEC_BLK_SIZE = -1;
// 0 means disabled (default), the rc channel used for setting the mcs index
// otherwise
static constexpr auto WB_MCS_INDEX_VIA_RC_CHANNEL_OFF = 0;
static constexpr auto WB_BW_VIA_RC_CHANNEL_OFF = 0;
static constexpr auto WB_TX_MODE_VIA_RC_CHANNEL_OFF = 0;
static constexpr auto WB_FHSS_VIA_RC_CHANNEL_OFF = 0;

static constexpr auto MAX_WIFI_CARDS = 4;

struct WBLinkSettings {
  uint32_t wb_frequency;  // writen once 2.4 or 5 is known
  // Air TX channel width. Ground will follow this once it receives management
  // frames from the air unit.
  uint32_t wb_air_tx_channel_width =
      DEFAULT_CHANNEL_WIDTH;  // 10 or 20 or 40 mhz bandwidth
  // Ground RX channel width when not yet synchronized with the air unit.
  uint32_t wb_gnd_rx_channel_width = DEFAULT_GND_RX_CHANNEL_WIDTH;
  // MCS index used during injection - only used by air unit, since ground
  // always sends with MCS0 unless manually overridden.
  uint32_t wb_air_mcs_index = DEFAULT_MCS_INDEX;
  // Ground uplink MCS index. Defaults to MCS0 for range/reliability.
  uint32_t wb_gnd_uplink_mcs_index = WB_GND_UPLINK_MCS_INDEX;
  int wb_enable_stbc = 0;  // 0==disabled
  bool wb_enable_ldpc = DEFAULT_ENABLE_LDPC;
  bool wb_enable_short_guard = DEFAULT_ENABLE_SHORT_GUARD;
  uint32_t wb_tx_power_milli_watt = DEFAULT_WIFI_TX_POWER_MILLI_WATT;
  uint32_t wb_tx_power_milli_watt_armed =
      WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED;
  // The only user-facing TX-power control: 20, 40, 60, 80 or 100 percent.
  int wb_tx_power_level = WB_TX_POWER_LEVEL_20;
  // rtl8812au driver does not support setting tx power by iw dev, but rather
  // only by setting a tx power index override param. With the most recent
  // openhd rtl8812au driver, we can even change this parameter dynamically. See
  // https://github.com/OpenHD/rtl8812au/blob/v5.2.20/os_dep/linux/ioctl_cfg80211.c#L3667
  // These values are the values that are passed to
  // NL80211_ATTR_WIPHY_TX_POWER_LEVEL this param is normally in mBm, but has
  // been reworked to accept those rtl8812au specific tx power index override
  // values (under this name they were known already in previous openhd
  // releases, but we now support changing them dynamcially at run time)
  uint32_t wb_rtl8812au_tx_pwr_idx_override = DEFAULT_RTL8812AU_TX_POWER_INDEX;
  // applied when armed
  uint32_t wb_rtl8812au_tx_pwr_idx_override_armed =
      RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED;
  // Per-card settings
  std::vector<int> wb_tx_power_mw_per_card;
  std::vector<int> wb_tx_power_mw_armed_per_card;
  std::vector<int> wb_tx_power_idx_per_card;
  std::vector<int> wb_tx_power_idx_armed_per_card;

  uint32_t wb_video_fec_percentage = DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  // decrease this value when there is a lot of pollution on your channel, and
  // you consistently get tx errors even though variable bitrate is working
  // fine. If you set this value to 80% (for example), it reduces the bitrate(s)
  // recommended to the encoder by 80% for each mcs index
  int wb_video_rate_for_mcs_adjustment_percent = 100;
  // NOTE: -1 means use whatever is the openhd recommendation for this platform
  int wb_max_fec_block_size = DEFAULT_MAX_FEC_BLK_SIZE;
  // Master switch for RC-driven OpenHD link controls.
  bool wb_enable_rc_openhd_control = false;
  // First of four consecutive channels for the clocked RC settings protocol.
  int wb_rc_settings_base_channel = 0;
  // change mcs index via RC channel
  uint32_t wb_mcs_index_via_rc_channel = WB_MCS_INDEX_VIA_RC_CHANNEL_OFF;
  // change bw via RC channel
  int wb_bw_via_rc_channel = WB_BW_VIA_RC_CHANNEL_OFF;
  // change tx mode via RC channel: low=off, mid=pit, high=normal
  int wb_tx_mode_via_rc_channel = WB_TX_MODE_VIA_RC_CHANNEL_OFF;
  // change fhss enable via RC channel: low=off, high=on
  int wb_fhss_via_rc_channel = WB_FHSS_VIA_RC_CHANNEL_OFF;
  // wb link recommends bitrate(s) to the encoder.
  bool enable_wb_video_variable_bitrate = true;
  int wb_qp_max = 17;
  int wb_qp_min = 42;
  // someone elses feed) but obviosuly you cannot reach your air unit anymore
  // when this mode is enabled (disable it to re-gain control)
  bool wb_enable_listen_only_mode = false;
  // Pit mode: when enabled, disarmed uses the 20% target.
  bool wb_pit_mode = true;
  // NOTE: Really complicated, for developers only
  bool wb_dev_air_set_high_retransmit_count = false;
  // Send same package on all connected cards
  bool wb_enable_redundant_tx = false;
  // Devourer Air only: reserve card 1 as a non-disruptive channel scout and
  // migrate the live link when a persistently cleaner channel is found.
  bool wb_enable_adaptive_channel = false;
  // Devourer owns the Wi-Fi FHSS clock/control channel. It is armed here but
  // only runs while an independent telemetry uplink (for example mLRS UART)
  // is confirmed live.
  bool wb_enable_fhss = false;
  int wb_fhss_slot_ms = 50;
  // Enable ARQ retransmission (standard: disabled)
  bool wb_enable_retransmission = false;
  // Enable ARQ retransmission for specific packet types
  bool wb_enable_retransmission_video = false;
  bool wb_enable_retransmission_telemetry = false;
  bool wb_enable_retransmission_rc = false;
  // Retransmission history window in milliseconds (per packet type)
  int wb_retransmission_history_video_ms = 10;
  int wb_retransmission_history_telemetry_ms = 10;
  int wb_retransmission_history_rc_ms = 10;
  // Retransmission request repeats
  int wb_retransmission_request_retries = 1;
};

WBLinkSettings create_default_wb_stream_settings(
    const std::vector<WiFiCard>& wifibroadcast_cards);

static bool validate_wb_rtl8812au_tx_pwr_idx_override(int value) {
  if (value >= 0 && value <= 63) return true;
  openhd::log::get_default()->warn(
      "Invalid wb_rtl8812au_tx_pwr_idx_override {}", value);
  return false;
}

class WBLinkSettingsHolder : public openhd::PersistentSettings<WBLinkSettings> {
 public:
  /**
   * @param platform needed to figure out the proper default params
   * @param wifibroadcast_cards1 needed to figure out the proper default params
   */
  explicit WBLinkSettingsHolder(OHDProfile profile,
                                std::vector<WiFiCard> wifibroadcast_cards1)
      : openhd::PersistentSettings<WBLinkSettings>(
            get_interface_settings_directory()),
        m_cards(std::move(wifibroadcast_cards1)),
        m_profile(std::move(profile)) {
    init();
  }

 public:
  const OHDProfile m_profile;
  const std::vector<WiFiCard> m_cards;

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    std::stringstream ss;
    ss << "wifibroadcast_settings.json";
    return ss.str();
  }
  [[nodiscard]] WBLinkSettings create_default() const override {
    return create_default_wb_stream_settings(m_cards);
  }

 private:
  std::optional<WBLinkSettings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const WBLinkSettings& data) const override;
};

// Note: max 16 char for id limit
static constexpr auto WB_FREQUENCY = "WB_FREQUENCY";
static constexpr auto WB_CHANNEL_WIDTH = "WB_CHANNEL_W";
static constexpr auto WB_MCS_INDEX = "WB_MCS_INDEX";
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH = "WB_V_FEC_BLK_L";
static constexpr auto WB_VIDEO_FEC_PERCENTAGE = "WB_V_FEC_PERC";
static constexpr auto WB_VIDEO_RATE_FOR_MCS_ADJUSTMENT_PERC =
    "WB_V_RATE_PERC";  // wb_video_rate_for_mcs_adjustment_percent
static constexpr auto WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM = "WB_MAX_D_BZ";
static constexpr auto WB_TX_POWER_MILLI_WATT = "TX_POWER_MW";
static constexpr auto WB_TX_POWER_MILLI_WATT_ARMED = "TX_POWER_MW_ARM";
static constexpr auto WB_TX_POWER_LEVEL = "TX_PWR_LVL";
// annoying 16 char settings limit
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_OVERRIDE = "TX_POWER_I";
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_ARMED = "TX_POWER_I_ARMED";
//
static constexpr auto WB_VIDEO_VARIABLE_BITRATE = "VARIABLE_BITRATE";
static constexpr auto WB_QP_MAX = "QP_MAX";
static constexpr auto WB_QP_MIN = "QP_MIN";
//
static constexpr auto WB_ENABLE_STBC = "WB_E_STBC";
static constexpr auto WB_ENABLE_LDPC = "WB_E_LDPC";
static constexpr auto WB_ENABLE_SHORT_GUARD = "WB_E_SHORT_GUARD";
static constexpr auto WB_ENABLE_RC_OPENHD_CONTROL = "RC_OHD_CTRL";
static constexpr auto WB_RC_SETTINGS_BASE_CHANNEL = "RC_SET_BASE";
static constexpr auto WB_MCS_INDEX_VIA_RC_CHANNEL = "MCS_VIA_RC";
static constexpr auto WB_BW_VIA_RC_CHANNEL = "BW_VIA_RC";
static constexpr auto WB_TX_MODE_VIA_RC_CHANNEL = "TXMODE_VIA_RC";
static constexpr auto WB_FHSS_VIA_RC_CHANNEL = "FHSS_VIA_RC";
static constexpr auto WB_PASSIVE_MODE = "WB_PASSIVE_MODE";
static constexpr auto WB_PIT_MODE = "WB_PIT_MODE";
static constexpr auto WB_DEV_AIR_SET_HIGH_RETRANSMIT_COUNT = "DEV_HIGH_RETR";
static constexpr auto WB_ENABLE_REDUNDANT_TX = "WB_RED_TX";
static constexpr auto WB_ENABLE_ADAPTIVE_CHANNEL = "WB_ADAPT_CH";
static constexpr auto WB_ENABLE_FHSS = "WB_FHSS";
static constexpr auto WB_FHSS_SLOT_MS = "WB_FHSS_SLOT";
static constexpr auto WB_ENABLE_RETRANSMISSION = "WB_ENABLE_RETRA";
static constexpr auto WB_ENABLE_RETRANSMISSION_VIDEO = "WB_RTX_VIDEO";
static constexpr auto WB_ENABLE_RETRANSMISSION_TELEMETRY = "WB_RTX_TELEM";
static constexpr auto WB_ENABLE_RETRANSMISSION_RC = "WB_RTX_RC";
static constexpr auto WB_RETRANSMISSION_HISTORY_VIDEO_MS = "WB_RTX_V_MAXMS";
static constexpr auto WB_RETRANSMISSION_HISTORY_TELEMETRY_MS = "WB_RTX_T_MAXMS";
static constexpr auto WB_RETRANSMISSION_HISTORY_RC_MS = "WB_RTX_R_MAXMS";
static constexpr auto WB_RETRANSMISSION_REQUEST_RETRIES = "WB_RTX_REQ_REP";

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
