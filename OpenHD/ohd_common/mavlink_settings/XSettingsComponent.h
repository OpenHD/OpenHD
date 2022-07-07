//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_

#include <utility>
#include <variant>
#include <string>
#include <vector>
#include <iostream>

namespace openhd{

using SettingsVariant= std::variant<int,float,std::string>;

struct Setting{
  const std::string id;
  SettingsVariant value;
};

class XSettingsComponent{
 public:
  // all the settings this component uses
  virtual std::vector<Setting> get_all_settings()=0;
  // perform required steps when a setting value changes.
  virtual void process_setting_changed(const Setting& changed_setting)=0;
};

/*bool safe_to(std::bool& value,const SettingsVariant& settings_variant){
  if(std::holds_alternative<bool>(settings_variant)){
    const auto new_value=std::get<bool>(settings_variant);
    if(new_value!=value){
      value=new_value;
      return true;
    }
  }
  return false;
}*/
static bool safe_to(int& value,const SettingsVariant& settings_variant){
  if(std::holds_alternative<int>(settings_variant)){
    const auto new_value=std::get<int>(settings_variant);
    if(new_value!=value){
      value=new_value;
      return true;
    }
  }
  return false;
}
static bool safe_to(float& value,const SettingsVariant& settings_variant){
  if(std::holds_alternative<float>(settings_variant)){
    const auto new_value=std::get<float>(settings_variant);
    if(new_value!=value){
      value=new_value;
      return true;
    }
  }
  return false;
}
static bool safe_to(std::string& value,const SettingsVariant& settings_variant){
  if(std::holds_alternative<std::string>(settings_variant)){
    const auto new_value=std::get<std::string>(settings_variant);
    if(new_value!=value){
      value=new_value;
      return true;
    }
  }
  return false;
}

// For testing
class DummyXSettingsComponent:public XSettingsComponent{
 public:
  std::vector<Setting> get_all_settings() override{
    std::vector<openhd::Setting> ret={
        openhd::Setting{"VIDEO_WIDTH",11},
        openhd::Setting{"VIDEO_HEIGHT",12},
        openhd::Setting{"VIDEO_FPS",13},
        openhd::Setting{"VIDEO_FORMAT","some"}
    };
    return ret;
  }
  void process_setting_changed(const Setting& changed_setting) override{
    std::cout<<"Setting: "<<changed_setting.id<<" changed\n";
  }
};


}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_
