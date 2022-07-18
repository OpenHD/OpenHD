//
// Created by consti10 on 17.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_

#include "openhd-wifi.hpp"

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
class WBStreamsSettingsHolder{
 public:
  explicit WBStreamsSettingsHolder(std::vector<WiFiCard> wifibroadcast_cards1):
	  wifibroadcast_cards(std::move(wifibroadcast_cards1)){
	if(!OHDFilesystemUtil::exists(WIFI_SETTINGS_DIRECTORY.c_str())){
	  OHDFilesystemUtil::create_directory(WIFI_SETTINGS_DIRECTORY);
	}
	const auto last_settings_opt=read_last_settings();
	if(last_settings_opt.has_value()){
	  _settings=std::make_unique<WBStreamsSettings>(last_settings_opt.value());
	  std::cout<<"Found settings in:"<<get_unique_filename()<<"\n";
	}else{
	  std::cout<<"Creating default settings:"<<get_unique_filename()<<"\n";
	  // create default settings and persist them for the next reboot
	  _settings=std::make_unique<WBStreamsSettings>(create_default_settings1(wifibroadcast_cards));
	  persist_settings();
	}
  }
  // delete copy and move constructor
  WBStreamsSettingsHolder(const WifiCardHolder&)=delete;
  WBStreamsSettingsHolder(const WifiCardHolder&&)=delete;
 public:
  const std::vector<WiFiCard> wifibroadcast_cards;
  [[nodiscard]] const WBStreamsSettings& get_settings()const{
	assert(_settings);
	return *_settings;
  }
  // unsafe becasue you then have to remember to persist the settings.
  WBStreamsSettings& unsafe_get_settings(){
	assert(_settings);
	return *_settings;
  }
  void persist(){
	persist_settings();
  }
 private:
  std::unique_ptr<WBStreamsSettings> _settings;
  [[nodiscard]] static std::string get_uniqe_hash(){
	std::stringstream ss;
	ss<<"interface_settings.json";
	return ss.str();
  }
  [[nodiscard]] static std::string get_unique_filename(){
	return INTERFACE_SETTINGS_DIRECTORY+get_uniqe_hash();
  }
  // write settings locally for persistence
  void persist_settings()const{
	assert(_settings);
	const auto filename= get_unique_filename();
	const nlohmann::json tmp=*_settings;
	// and write them locally for persistence
	std::ofstream t(filename);
	t << tmp.dump(4);
	t.close();
  }
  // read last settings, if they are available
  [[nodiscard]] std::optional<WBStreamsSettings> read_last_settings()const{
	const auto filename= get_unique_filename();
	if(!OHDFilesystemUtil::exists(filename.c_str())){
	  return std::nullopt;
	}
	std::ifstream f(filename);
	nlohmann::json j;
	f >> j;
	return j.get<WBStreamsSettings>();
  }
};

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_OHDINTERFACESETTINGS_H_
