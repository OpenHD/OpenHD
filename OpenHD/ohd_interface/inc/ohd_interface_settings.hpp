//
// Created by consti10 on 19.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_INTERFACE_SETTINGS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_INTERFACE_SETTINGS_HPP_

#include "openhd-settings2.hpp"
#include "wb_link_settings.hpp"

namespace openhd{

// NOTE: wb_link intentionally has its own settings
struct OHDInterfaceSettings {
  bool enable_wifi_hotspot=false;
  uint32_t hotspot_wifi_channel=8;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OHDInterfaceSettings,enable_wifi_hotspot,hotspot_wifi_channel);

class OHDInterfaceSettingsHolder:public openhd::settings::PersistentSettings<OHDInterfaceSettings>{
 public:
  OHDInterfaceSettingsHolder():
	  openhd::settings::PersistentSettings<OHDInterfaceSettings>(INTERFACE_SETTINGS_DIRECTORY){
	init();
  }
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<"interface_settings.json";
	return ss.str();
  }
  [[nodiscard]] OHDInterfaceSettings create_default()const override{
	return OHDInterfaceSettings{};
  }
};

}
#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_INTERFACE_SETTINGS_HPP_
