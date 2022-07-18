//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_

#include "openhd-wifi.hpp"
#include "openhd-settings2.hpp"

namespace openhd{

struct WBStreamsSettings{
  uint32_t wb_frequency=5180;
  uint32_t wb_channel_bandwidth=20; // 20 or 40 mhz bandwidth
  uint32_t wb_mcs_index=5;
  uint32_t wb_video_fec_block_length=8;
  uint32_t wb_video_fec_percentage=20;
  uint32_t wb_tx_power=1800;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WBStreamsSettings,wb_frequency,wb_channel_bandwidth,wb_mcs_index,
								   wb_video_fec_block_length,wb_video_fec_percentage,wb_tx_power);

static WBStreamsSettings create_default_settings1(const std::vector<WiFiCard>& wifibroadcast_cards){
  assert(!wifibroadcast_cards.empty());
  const auto first_card=wifibroadcast_cards.at(0);
  assert(first_card.supports_5ghz || first_card.supports_2ghz);
  const bool use_5ghz= wifibroadcast_cards.at(0).supports_5ghz;
  WBStreamsSettings settings{};
  if(use_5ghz){
	settings.wb_frequency=5180;
  }else{
	settings.wb_frequency=2412;
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
	return create_default_settings1(wifibroadcast_cards);
  }
};

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_
