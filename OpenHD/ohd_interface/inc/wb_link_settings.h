//
// Created by consti10 on 17.07.22.
//

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
// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that
// ever made it trough though) This value seems a bit high to me, so I am going
// with a default of "1800" (which should be 18.0 dBm ) Used to be in dBm, but
// mW really is more verbose to the user - we convert from mW to dBm when using
// the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT = 25;
// by default, we do not differentiate (to not confuse the user)
static constexpr auto WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED = 0;
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

struct WBLinkSettings {
  uint32_t wb_frequency;  // writen once 2.4 or 5 is known
  // NOTE: Only stored on air, gnd automatically applies 40Mhz bwidth when air
  // reports (management frame(s))
  uint32_t wb_air_tx_channel_width =
      DEFAULT_CHANNEL_WIDTH;  // 20 or 40 mhz bandwidth
  // MCS index used during injection - only used by air unit, since ground
  // always sends with MCS0
  uint32_t wb_air_mcs_index = DEFAULT_MCS_INDEX;
  int wb_enable_stbc = 0;  // 0==disabled
  bool wb_enable_ldpc = DEFAULT_ENABLE_LDPC;
  bool wb_enable_short_guard = DEFAULT_ENABLE_SHORT_GUARD;
  uint32_t wb_tx_power_milli_watt = DEFAULT_WIFI_TX_POWER_MILLI_WATT;
  uint32_t wb_tx_power_milli_watt_armed =
      WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED;
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
  uint32_t wb_video_fec_percentage = DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  // decrease this value when there is a lot of pollution on your channel, and
  // you consistently get tx errors even though variable bitrate is working
  // fine. If you set this value to 80% (for example), it reduces the bitrate(s)
  // recommended to the encoder by 80% for each mcs index
  int wb_video_rate_for_mcs_adjustment_percent = 100;
  // NOTE: -1 means use whatever is the openhd recommendation for this platform
  int wb_max_fec_block_size = DEFAULT_MAX_FEC_BLK_SIZE;
  // change mcs index via RC channel
  uint32_t wb_mcs_index_via_rc_channel = WB_MCS_INDEX_VIA_RC_CHANNEL_OFF;
  // change bw via RC channel
  int wb_bw_via_rc_channel = WB_BW_VIA_RC_CHANNEL_OFF;
  // wb link recommends bitrate(s) to the encoder.
  bool enable_wb_video_variable_bitrate = true;
  // !!!!
  // This allows the ground station to become completely passive (aka tune in on
  // someone elses feed) but obviosuly you cannot reach your air unit anymore
  // when this mode is enabled (disable it to re-gain control)
  bool wb_enable_listen_only_mode = false;
  // NOTE: Really complicated, for developers only
  bool wb_dev_air_set_high_retransmit_count = false;
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
// annoying 16 char settings limit
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_OVERRIDE = "TX_POWER_I";
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_ARMED = "TX_POWER_I_ARMED";
//
static constexpr auto WB_VIDEO_VARIABLE_BITRATE = "VARIABLE_BITRATE";
//
static constexpr auto WB_ENABLE_STBC = "WB_E_STBC";
static constexpr auto WB_ENABLE_LDPC = "WB_E_LDPC";
static constexpr auto WB_ENABLE_SHORT_GUARD = "WB_E_SHORT_GUARD";
static constexpr auto WB_MCS_INDEX_VIA_RC_CHANNEL = "MCS_VIA_RC";
static constexpr auto WB_BW_VIA_RC_CHANNEL = "BW_VIA_RC";
static constexpr auto WB_PASSIVE_MODE = "WB_PASSIVE_MODE";
static constexpr auto WB_DEV_AIR_SET_HIGH_RETRANSMIT_COUNT = "DEV_HIGH_RETR";

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
