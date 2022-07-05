//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_

#include <utility>
#include <variant>
#include <string>
#include <vector>

namespace openhd{

using SettingsVariant= std::variant<bool,int,float,std::string>;

struct Setting{
  const std::string id;
  SettingsVariant value;
};

class XSettingsComponent{
 public:
  // all the settings this component param server provides
  virtual std::vector<Setting> get_all_settings()=0;
  // perform required steps when a setting value changes.
  virtual void process_setting_changed(const Setting& changed_setting)=0;
};

}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_
