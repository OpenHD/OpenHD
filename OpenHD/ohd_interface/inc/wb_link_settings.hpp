//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_

#include "openhd-settings2.hpp"
#include "wifi_card.hpp"

namespace openhd{

// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that ever made it trough though)
// This value seems a bit high to me, so I am going with a default of "1800" (which should be 18.0 dBm )
// Used to be in dBm, but mW really is more verbose to the user - we convert from mW to dBm when using the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT=25;
static constexpr auto DEFAULT_5GHZ_FREQUENCY = 5180;
static constexpr auto DEFAULT_2GHZ_FREQUENCY = 2412;

static constexpr auto DEFAULT_MCS_INDEX=3;
static constexpr auto DEFAULT_CHANNEL_WIDTH=20;
// Set to 0 for fec auto block length
// Set to 1 or greater for fixed k fec
static constexpr auto DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH=12;
static constexpr auto DEFAULT_WB_VIDEO_FEC_PERCENTAGE=50;

struct WBLinkSettings {
  uint32_t wb_frequency; // writen once 2.4 or 5 is known
  uint32_t wb_channel_width=DEFAULT_CHANNEL_WIDTH; // 20 or 40 mhz bandwidth
  uint32_t wb_mcs_index=DEFAULT_MCS_INDEX;
  bool wb_enable_stbc=false;
  bool wb_enable_ldpc=false;
  bool wb_enable_short_guard=false;
  //
  uint32_t wb_video_fec_block_length=DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH;
  uint32_t wb_video_fec_percentage=DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  uint32_t wb_tx_power_milli_watt=DEFAULT_WIFI_TX_POWER_MILLI_WATT;
  bool enable_wb_video_variable_bitrate= false;// wb link recommends bitrate(s) to the encoder, can be helpfully for inexperienced users.
  [[nodiscard]] bool configured_for_2G()const{
	return is_2G_and_assert(wb_frequency);
  }
  bool is_video_variable_block_length_enabled()const{
    return wb_video_fec_block_length==0;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBLinkSettings, wb_frequency, wb_channel_width, wb_mcs_index,
                                   wb_video_fec_block_length, wb_video_fec_percentage, wb_tx_power_milli_watt,
                                   enable_wb_video_variable_bitrate,
                                   wb_enable_stbc,wb_enable_ldpc,wb_enable_short_guard);

static WBLinkSettings create_default_wb_stream_settings(const std::vector<WiFiCard>& wifibroadcast_cards){
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
  return settings;
}

static std::vector<WiFiCard> tmp_convert(const std::vector<std::shared_ptr<WifiCardHolder>>& broadcast_cards){
  std::vector<WiFiCard> ret;
  for(const auto& holder:broadcast_cards){
	ret.push_back(holder->_wifi_card);
  }
  return ret;
}

static const std::string INTERFACE_SETTINGS_DIRECTORY=std::string(BASE_PATH)+std::string("interface/");

class WBStreamsSettingsHolder:public openhd::settings::PersistentSettings<WBLinkSettings>{
 public:
  explicit WBStreamsSettingsHolder(std::vector<WiFiCard> wifibroadcast_cards1):
	  openhd::settings::PersistentSettings<WBLinkSettings>(INTERFACE_SETTINGS_DIRECTORY),
	  wifibroadcast_cards(std::move(wifibroadcast_cards1)){
	init();
  }
 public:
  const std::vector<WiFiCard> wifibroadcast_cards;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<"wifibroadcast_settings.json";
	return ss.str();
  }
  [[nodiscard]] WBLinkSettings create_default()const override{
	return create_default_wb_stream_settings(wifibroadcast_cards);
  }
};

// Note: max 16 char for id limit
static constexpr auto WB_FREQUENCY="WB_FREQUENCY";
static constexpr auto WB_CHANNEL_WIDTH="WB_CHANNEL_W";
static constexpr auto WB_MCS_INDEX="WB_MCS_INDEX";
static constexpr auto WB_VIDEO_FEC_BLOCK_LENGTH="WB_V_FEC_BLK_L";
static constexpr auto WB_VIDEO_FEC_PERCENTAGE="WB_V_FEC_PERC";
static constexpr auto WB_TX_POWER_MILLI_WATT="WB_TX_POWER_MW";
//
static constexpr auto WB_VIDEO_VARIABLE_BITRATE="VARIABLE_BITRATE";
//
static constexpr auto WB_ENABLE_STBC="WB_E_STBC";
static constexpr auto WB_ENABLE_LDPC="WB_E_LDPC";
static constexpr auto WB_ENABLE_SHORT_GUARD="WB_E_SHORT_GUARD";

}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_SETTINGS_HPP_
