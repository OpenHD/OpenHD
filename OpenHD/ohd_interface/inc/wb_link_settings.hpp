//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_

#include <utility>

#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_persistent.h"
#include "openhd_settings_directories.hpp"
#include "wifi_card.h"

namespace openhd{

static constexpr auto DEFAULT_5GHZ_FREQUENCY = 5180;
static constexpr auto DEFAULT_2GHZ_FREQUENCY = 2412;
static constexpr auto DEFAULT_MCS_INDEX=3;
// We always use a MCS index of X for the uplink, since (compared to the video link) it requires a negligible amount of bandwidth
// and for those using RC over OpenHD, we have the benefit that the range of RC is "more" than the range for video
static constexpr auto DEFAULT_GND_UPLINK_MCS_INDEX=0;
static constexpr auto DEFAULT_CHANNEL_WIDTH=20;
// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that ever made it trough though)
// This value seems a bit high to me, so I am going with a default of "1800" (which should be 18.0 dBm )
// Used to be in dBm, but mW really is more verbose to the user - we convert from mW to dBm when using the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT=25;
// Measured to be about /below 25mW, RTL8812au only (or future cards who use the recommended power level index approach)
static constexpr auto DEFAULT_RTL8812AU_TX_POWER_INDEX=22;
// by default, we do not differentiate (to not confuse users)
static constexpr auto RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED=0;

// Set to 0 for fec auto block length
// Set to 1 or greater for fixed k fec
// Default to auto since 2.2.5-evo
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH_AUTO=0;
static constexpr auto DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH=WB_VIDEO_FEC_BLOCK_LENGTH_AUTO;
// FEC can fixup packet loss, as long as is statistically well distributed (no big gaps)
// if there are many big gaps, increasing the FEC percentage often doesn't help, it is better to reduce the key frame interval
// of your camera in this case
static constexpr auto DEFAULT_WB_VIDEO_FEC_PERCENTAGE=20;
//NOTE: Default depends on platform type and is therefore calculated below, then overwrites this default value
static constexpr uint32_t DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM=20;
// 0 means disabled (default), the rc channel used for setting the mcs index otherwise
static constexpr auto WB_MCS_INDEX_VIA_RC_CHANNEL_OFF=0;

struct WBLinkSettings {
  uint32_t wb_frequency; // writen once 2.4 or 5 is known
  uint32_t wb_channel_width=DEFAULT_CHANNEL_WIDTH; // 20 or 40 mhz bandwidth
  uint32_t wb_mcs_index=DEFAULT_MCS_INDEX;
  int wb_enable_stbc=0; // 0==disabled
  bool wb_enable_ldpc=false;
  bool wb_enable_short_guard=false;
  uint32_t wb_tx_power_milli_watt=DEFAULT_WIFI_TX_POWER_MILLI_WATT;
  // rtl8812au driver does not support setting tx power by iw dev, but rather only by setting
  // a tx power index override param. With the most recent openhd rtl8812au driver,
  // we can even change this parameter dynamically.
  // See https://github.com/OpenHD/rtl8812au/blob/v5.2.20/os_dep/linux/ioctl_cfg80211.c#L3667
  // These values are the values that are passed to NL80211_ATTR_WIPHY_TX_POWER_LEVEL
  // this param is normally in mBm, but has been reworked to accept those rtl8812au specific tx power index override values
  // (under this name they were known already in previous openhd releases, but we now support changing them dynamcially at run time)
  uint32_t wb_rtl8812au_tx_pwr_idx_override=DEFAULT_RTL8812AU_TX_POWER_INDEX;
  // applied when armed
  uint32_t wb_rtl8812au_tx_pwr_idx_override_armed=RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED;
  uint32_t wb_video_fec_percentage=DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  // decrease this value when there is a lot of pollution on your channel, and you consistently get tx errors
  // even though variable bitrate is working fine.
  // If you set this value to 80% (for example), it reduces the bitrate(s) recommended to the encoder by 80% for each mcs index
  int wb_video_rate_for_mcs_adjustment_percent=100;
  // NOTE: Default depends on platform type and is therefore calculated below, then overwrites this default value
  uint32_t wb_max_fec_block_size_for_platform=DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM;
  // change mcs index via RC channel
  uint32_t wb_mcs_index_via_rc_channel=WB_MCS_INDEX_VIA_RC_CHANNEL_OFF;

  // wb link recommends bitrate(s) to the encoder, can be helpfully for inexperienced users.
  bool enable_wb_video_variable_bitrate= true;
  // !!!!
  // This allows the ground station to become completely passive (aka tune in on someone elses feed)
  // but obviosuly you cannot reach your air unit anymore when this mode is enabled
  // (disable it to re-gain control)
  bool wb_enable_listen_only_mode= false;
  // Enable / Disable video encryption - off by default. Telemetry is always encrypted
  // This setting is only valid on the air unit, where video is encrypted (on the ground, it is ignored) - it does not need to macth
  // we have per-packet enable / disable encryption in wifibroadcast
  bool wb_air_enable_video_encryption= false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBLinkSettings, wb_frequency, wb_channel_width, wb_mcs_index,
                                   wb_enable_stbc, wb_enable_ldpc, wb_enable_short_guard,
                                   wb_tx_power_milli_watt, wb_rtl8812au_tx_pwr_idx_override, wb_rtl8812au_tx_pwr_idx_override_armed,
                                   wb_video_fec_percentage,
                                   wb_video_rate_for_mcs_adjustment_percent,
                                   wb_max_fec_block_size_for_platform,
                                   wb_mcs_index_via_rc_channel,
                                   enable_wb_video_variable_bitrate,
                                   wb_enable_listen_only_mode,
                                   wb_air_enable_video_encryption);

static int calculate_max_fec_block_size_for_platform(const OHDPlatform platform){
  switch (platform.platform_type) {
    case PlatformType::RaspberryPi:{
      if(platform_rpi_is_high_performance(platform)){
        return 30;
      }
      return 20;
    }
      break;
    case PlatformType::PC: // x86 is so much more powerful than ARM so we can easily do 30, assuming no one uses HW from 2010 ;)
      return 30;
    case PlatformType::Jetson:
    case PlatformType::Allwinner:
    case PlatformType::iMX6:
    case PlatformType::Rockchip:
    case PlatformType::Zynq:
    case PlatformType::Unknown:
    default:
      return 20;
  }
  return 20;
}

static WBLinkSettings create_default_wb_stream_settings(const OHDPlatform& platform,const OHDProfile& profile,const std::vector<WiFiCard>& wifibroadcast_cards){
  assert(!wifibroadcast_cards.empty());
  const auto first_card=wifibroadcast_cards.at(0);
  assert(first_card.supports_5GHz() || first_card.supports_2GHz());
  const bool use_5ghz= wifibroadcast_cards.at(0).supports_5GHz();
  WBLinkSettings settings{};
  if(use_5ghz){
	settings.wb_frequency=DEFAULT_5GHZ_FREQUENCY;
  }else{
	settings.wb_frequency=DEFAULT_2GHZ_FREQUENCY;
  }
  settings.wb_max_fec_block_size_for_platform= calculate_max_fec_block_size_for_platform(platform);
  if(all_cards_support_setting_mcs_index(wifibroadcast_cards) && profile.is_ground()){
	settings.wb_mcs_index=DEFAULT_GND_UPLINK_MCS_INDEX;
  }
  openhd::log::get_default()->debug("Default wb_max_fec_block_size_for_platform:{}",settings.wb_max_fec_block_size_for_platform);
  return settings;
}

static bool validate_wb_rtl8812au_tx_pwr_idx_override(int value){
  if(value>=0 && value <= 63)return true;
  openhd::log::get_default()->warn("Invalid wb_rtl8812au_tx_pwr_idx_override {}",value);
  return false;
}

static void write_modprobe_file_rtl8812au_wb(int rtw_tx_pwr_idx_override){
  std::stringstream ss;
  ss<<"options 88XXau_wfb "<<"rtw_tx_pwr_idx_override="<<rtw_tx_pwr_idx_override<<"\n";
  OHDFilesystemUtil::write_file("/etc/modprobe.d/88XXau_wfb.conf",ss.str());
}

// We allow the user to overwrite defaults for his platform.
// The FEC impl limit would be 128 - but anything above 50 is not computable on any platform
static bool valid_wb_max_fec_block_size_for_platform(uint32_t wb_max_fec_block_size_for_platform){
  return wb_max_fec_block_size_for_platform>0 && wb_max_fec_block_size_for_platform<=100;
}

class WBStreamsSettingsHolder:public openhd::PersistentJsonSettings<WBLinkSettings>{
 public:
  /**
   * @param platform needed to figure out the proper default params
   * @param wifibroadcast_cards1 needed to figure out the proper default params
   */
  explicit WBStreamsSettingsHolder(OHDPlatform platform,OHDProfile profile,std::vector<WiFiCard> wifibroadcast_cards1):
	  openhd::PersistentJsonSettings<WBLinkSettings>(get_interface_settings_directory()),
        m_cards(std::move(wifibroadcast_cards1)),
          m_platform(platform),
		  m_profile(std::move(profile))
  {
	init();
  }
 public:
  const OHDPlatform m_platform;
  const OHDProfile m_profile;
  const std::vector<WiFiCard> m_cards;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<"wifibroadcast_settings.json";
	return ss.str();
  }
  [[nodiscard]] WBLinkSettings create_default()const override{
	return create_default_wb_stream_settings(m_platform,m_profile, m_cards);
  }
};

// Note: max 16 char for id limit
static constexpr auto WB_FREQUENCY="WB_FREQUENCY";
static constexpr auto WB_CHANNEL_WIDTH="WB_CHANNEL_W";
static constexpr auto WB_MCS_INDEX="WB_MCS_INDEX";
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH="WB_V_FEC_BLK_L";
static constexpr auto WB_VIDEO_FEC_PERCENTAGE="WB_V_FEC_PERC";
static constexpr auto WB_VIDEO_RATE_FOR_MCS_ADJUSTMENT_PERC="WB_V_RATE_PERC"; //wb_video_rate_for_mcs_adjustment_percent
static constexpr auto WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM="WB_MAX_D_BZ";
static constexpr auto WB_TX_POWER_MILLI_WATT="TX_POWER_MW";
static constexpr auto WB_VIDEO_ENCRYPTION_ENABLE="WB_VIDEO_ENCRYPT";
// annoying 16 char settings limit
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_OVERRIDE="TX_POWER_I";
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_ARMED="TX_POWER_I_ARMED";
//
static constexpr auto WB_VIDEO_VARIABLE_BITRATE="VARIABLE_BITRATE";
//
static constexpr auto WB_ENABLE_STBC="WB_E_STBC";
static constexpr auto WB_ENABLE_LDPC="WB_E_LDPC";
static constexpr auto WB_ENABLE_SHORT_GUARD="WB_E_SHORT_GUARD";
static constexpr auto WB_MCS_INDEX_VIA_RC_CHANNEL="MCS_VIA_RC";
static constexpr auto WB_PASSIVE_MODE ="WB_PASSIVE_MODE";

}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
