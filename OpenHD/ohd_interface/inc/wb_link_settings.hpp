//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_

#include "openhd-settings2.hpp"
#include "wifi_card.hpp"
#include "openhd-platform.hpp"

namespace openhd{

static constexpr auto DEFAULT_5GHZ_FREQUENCY = 5180;
static constexpr auto DEFAULT_2GHZ_FREQUENCY = 2412;
static constexpr auto DEFAULT_MCS_INDEX=3;
static constexpr auto DEFAULT_CHANNEL_WIDTH=20;
// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that ever made it trough though)
// This value seems a bit high to me, so I am going with a default of "1800" (which should be 18.0 dBm )
// Used to be in dBm, but mW really is more verbose to the user - we convert from mW to dBm when using the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT=25;
// Measured to be slightly below 25mW, RTL8812au only (or future cards who use the recommended power level index approach)
static constexpr auto DEFAULT_RTL8812AU_TX_POWER_INDEX=19;

// Set to 0 for fec auto block length
// Set to 1 or greater for fixed k fec
// Default to auto since 2.2.5-evo
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH_AUTO=0;
static constexpr auto DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH=WB_VIDEO_FEC_BLOCK_LENGTH_AUTO;
static constexpr auto DEFAULT_WB_VIDEO_FEC_PERCENTAGE=50;
//NOTE: Default depends on platform type and is therefore calculated below, then overwrites this default value
static constexpr uint32_t DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM=20;

struct WBLinkSettings {
  uint32_t wb_frequency; // writen once 2.4 or 5 is known
  uint32_t wb_channel_width=DEFAULT_CHANNEL_WIDTH; // 20 or 40 mhz bandwidth
  uint32_t wb_mcs_index=DEFAULT_MCS_INDEX;
  bool wb_enable_stbc=false;
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
  // 0 means auto, aka variable block size (default, gives best results in most cases and has 0 additional latency)
  uint32_t wb_video_fec_block_length=DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH;
  uint32_t wb_video_fec_percentage=DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  // NOTE: Default depends on platform type and is therefore calculated below, then overwrites this default value
  uint32_t wb_max_fec_block_size_for_platform=DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM;

  // wb link recommends bitrate(s) to the encoder, can be helpfully for inexperienced users.
  bool enable_wb_video_variable_bitrate= false;

  // Helper
  [[nodiscard]] bool configured_for_2G()const{
	return is_2G_and_assert(wb_frequency);
  }
  bool is_video_variable_block_length_enabled()const{
    return wb_video_fec_block_length==0;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBLinkSettings, wb_frequency, wb_channel_width, wb_mcs_index,
                                   wb_enable_stbc,wb_enable_ldpc,wb_enable_short_guard,
                                   wb_tx_power_milli_watt,wb_rtl8812au_tx_pwr_idx_override,
                                   wb_video_fec_block_length, wb_video_fec_percentage,wb_max_fec_block_size_for_platform,
                                   enable_wb_video_variable_bitrate);

static int calculate_max_fec_block_size_for_platform(const OHDPlatform platform){
  switch (platform.platform_type) {
    case PlatformType::RaspberryPi:{
      if(platform_rpi_is_high_performance(platform)){
        return 30;
      }
      return 20;
    }
      break;
    case PlatformType::PC:
    case PlatformType::Jetson:
    case PlatformType::NanoPi:
    case PlatformType::iMX6:
    case PlatformType::Rockchip:
    case PlatformType::Zynq:
    case PlatformType::Unknown:
    default:
      return 20;
  }
  return 20;
}

static WBLinkSettings create_default_wb_stream_settings(const OHDPlatform& platform,const std::vector<WiFiCard>& wifibroadcast_cards){
  assert(!wifibroadcast_cards.empty());
  const auto first_card=wifibroadcast_cards.at(0);
  assert(first_card.supports_5ghz || first_card.supports_2ghz);
  const bool use_5ghz= wifibroadcast_cards.at(0).supports_5ghz;
  WBLinkSettings settings{};
  if(use_5ghz){
	settings.wb_frequency=DEFAULT_5GHZ_FREQUENCY;
  }else{
	settings.wb_frequency=DEFAULT_2GHZ_FREQUENCY;
  }
  settings.wb_max_fec_block_size_for_platform= calculate_max_fec_block_size_for_platform(platform);
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
  return wb_max_fec_block_size_for_platform>0 && wb_max_fec_block_size_for_platform<50;
}

static std::vector<WiFiCard> tmp_convert(const std::vector<std::shared_ptr<WifiCardHolder>>& broadcast_cards){
  std::vector<WiFiCard> ret;
  for(const auto& holder:broadcast_cards){
	ret.push_back(holder->_wifi_card);
  }
  return ret;
}

class WBStreamsSettingsHolder:public openhd::settings::PersistentSettings<WBLinkSettings>{
 public:
  /**
   * @param platform needed to figure out the proper default params
   * @param wifibroadcast_cards1 needed to figure out the proper default params
   */
  explicit WBStreamsSettingsHolder(OHDPlatform platform,std::vector<WiFiCard> wifibroadcast_cards1):
	  openhd::settings::PersistentSettings<WBLinkSettings>(INTERFACE_SETTINGS_DIRECTORY),
	  wifibroadcast_cards(std::move(wifibroadcast_cards1)),
          m_platform(platform)
  {
	init();
  }
  // set default 2G channel and channel width
  void set_default_2G(){
    unsafe_get_settings().wb_frequency=openhd::DEFAULT_2GHZ_FREQUENCY;
    unsafe_get_settings().wb_channel_width=openhd::DEFAULT_CHANNEL_WIDTH;
    persist();
  }
  void set_default_5G(){
    unsafe_get_settings().wb_frequency=openhd::DEFAULT_5GHZ_FREQUENCY;
    unsafe_get_settings().wb_channel_width=openhd::DEFAULT_CHANNEL_WIDTH;
    persist();
  }
 public:
  const OHDPlatform m_platform;
  const std::vector<WiFiCard> wifibroadcast_cards;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<"wifibroadcast_settings.json";
	return ss.str();
  }
  [[nodiscard]] WBLinkSettings create_default()const override{
	return create_default_wb_stream_settings(m_platform,wifibroadcast_cards);
  }
};

// Note: max 16 char for id limit
static constexpr auto WB_FREQUENCY="WB_FREQUENCY";
static constexpr auto WB_CHANNEL_WIDTH="WB_CHANNEL_W";
static constexpr auto WB_MCS_INDEX="WB_MCS_INDEX";
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH="WB_V_FEC_BLK_L";
static constexpr auto WB_VIDEO_FEC_PERCENTAGE="WB_V_FEC_PERC";
//static constexpr auto WB_VIDEO_FEC_DYNAMIC_MAX_BLOCK_SIZE="WB_V_FEC_D_MAX";
static constexpr auto WB_TX_POWER_MILLI_WATT="WB_TX_POWER_MW";
// annoying 16 char settings limit
static constexpr auto WB_RTL8812AU_TX_PWR_IDX_OVERRIDE="WB_TX_PWR_IDX_O";
//
static constexpr auto WB_VIDEO_VARIABLE_BITRATE="VARIABLE_BITRATE";
//
static constexpr auto WB_ENABLE_STBC="WB_E_STBC";
static constexpr auto WB_ENABLE_LDPC="WB_E_LDPC";
static constexpr auto WB_ENABLE_SHORT_GUARD="WB_E_SHORT_GUARD";


}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
