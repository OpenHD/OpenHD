//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WBSTREAMSSETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WBSTREAMSSETTINGS_HPP_

#include "OHDWifiCard.hpp"
#include "openhd-settings2.hpp"

namespace openhd{

// Consti10: Stephen used a default tx power of 3100 somewhere (not sure if that ever made it trough though)
// This value seems a bit high to me, so I am going with a default of "1800" (which should be 18.0 dBm )
// Used to be in dBm, but mW really is more verbose to the user - we convert from mW to dBm when using the iw dev set command
static constexpr auto DEFAULT_WIFI_TX_POWER_MILLI_WATT=25;
static constexpr auto DEFAULT_5GHZ_FREQUENCY = 5180;
static constexpr auto DEFAULT_2GHZ_FREQUENCY = 2412;

static constexpr auto DEFAULT_MCS_INDEX=3;
static constexpr auto DEFAULT_CHANNEL_WIDTH=20;
static constexpr auto DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH=12;
static constexpr auto DEFAULT_WB_VIDEO_FEC_PERCENTAGE=80;

struct WBStreamsSettings{
  uint32_t wb_frequency; // writen once 2.4 or 5 is known
  uint32_t wb_channel_width=DEFAULT_CHANNEL_WIDTH; // 20 or 40 mhz bandwidth
  uint32_t wb_mcs_index=DEFAULT_MCS_INDEX;
  uint32_t wb_video_fec_block_length=DEFAULT_WB_VIDEO_FEC_BLOCK_LENGTH;
  uint32_t wb_video_fec_percentage=DEFAULT_WB_VIDEO_FEC_PERCENTAGE;
  uint32_t wb_tx_power_milli_watt=DEFAULT_WIFI_TX_POWER_MILLI_WATT;
  bool wb_video_fec_block_length_auto_enable=false; // Adjust block size(s) to size of fragmented rtp video frame
  [[nodiscard]] bool configured_for_2G()const{
	return is_2G_and_assert(wb_frequency);
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBStreamsSettings, wb_frequency, wb_channel_width, wb_mcs_index,
								   wb_video_fec_block_length, wb_video_fec_percentage, wb_tx_power_milli_watt,wb_video_fec_block_length_auto_enable);

static WBStreamsSettings create_default_wb_stream_settings(const std::vector<WiFiCard>& wifibroadcast_cards){
  assert(!wifibroadcast_cards.empty());
  const auto first_card=wifibroadcast_cards.at(0);
  assert(first_card.supports_5ghz || first_card.supports_2ghz);
  const bool use_5ghz= wifibroadcast_cards.at(0).supports_5ghz;
  WBStreamsSettings settings{};
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

class WBStreamsSettingsHolder:public openhd::settings::PersistentSettings<WBStreamsSettings>{
 public:
  explicit WBStreamsSettingsHolder(std::vector<WiFiCard> wifibroadcast_cards1):
	  openhd::settings::PersistentSettings<WBStreamsSettings>(INTERFACE_SETTINGS_DIRECTORY),
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
  [[nodiscard]] WBStreamsSettings create_default()const override{
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
static constexpr auto WB_FEC_BLOCK_LENGTH_AUTO_ENABLE="WB_FEC_BL_AUTO";

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_WBSTREAMSSETTINGS_HPP_
