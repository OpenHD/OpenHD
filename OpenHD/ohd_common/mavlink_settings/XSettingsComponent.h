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
#include <map>
#include <cassert>
#include <functional>

namespace openhd{

template<class T>
struct SettingImpl{
  T value;
  // This callback is called every time the user wants to change the parameter from value x to value y (via mavlink)
  // return true to accept the value, otherwise return false.
  std::function<bool(std::string id,T requested_value)> change_callback=[](std::string id,T requested_value){
	std::cout<<id<<"change to "<<std::to_string(requested_value)<<"\n";
	return true;
  };
};
using IntSetting=SettingImpl<int>;
using FloatSetting=SettingImpl<float>;
using StringSetting=SettingImpl<std::string>;

struct Setting{
  const std::string id;
  std::variant<IntSetting,FloatSetting,StringSetting> setting;
};

class XSettingsComponent{
 public:
  // all the settings this component uses
  virtual std::vector<Setting> get_all_settings()=0;
 public:
  XSettingsComponent()=default;
  // delete copy and move constructor
  XSettingsComponent(const XSettingsComponent&)=delete;
  XSettingsComponent(const XSettingsComponent&&)=delete;
};


/*using SettingsVariant= std::variant<int,float,std::string>;
struct Setting{
  const std::string id;
  SettingsVariant value;
};

class XSettingsComponent{
 public:
  // all the settings this component uses
  virtual std::vector<Setting> get_all_settings()=0;
  // perform required steps when a setting value changes.
  virtual void process_setting_changed(Setting changed_setting)=0;
 public:
  XSettingsComponent()=default;
  // delete copy and move constructor
  XSettingsComponent(const XSettingsComponent&)=delete;
  XSettingsComponent(const XSettingsComponent&&)=delete;
};*/

static void validate_provided_ids(const std::vector<Setting>& settings){
  // we need to have unique setting string ids. If there is a duplicate, this would be a programmers error,
  // and when used correctly should be found during debugging by this method.
  std::map<std::string,void*> test;
  for(const auto& setting:settings){
	assert(test.find(setting.id)==test.end());
	test[setting.id]=nullptr;
  }
}

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
/*static bool safe_to(int& value,const SettingsVariant& settings_variant){
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
}*/

// For testing
namespace testing{
class DummyCameraXSettingsComponent:public XSettingsComponent{
 public:
  std::vector<Setting> get_all_settings() override{
    std::vector<openhd::Setting> ret={
        openhd::Setting{"VIDEO_WIDTH",openhd::IntSetting{640, nullptr}},
        openhd::Setting{"VIDEO_HEIGHT",openhd::IntSetting{480, nullptr}},
        openhd::Setting{"VIDEO_FPS",openhd::IntSetting{30, nullptr}},
        openhd::Setting{"VIDEO_FORMAT",openhd::IntSetting{0, nullptr}},
        openhd::Setting{"V_BITRATE_MBITS",openhd::IntSetting{10, nullptr}},
    };
    return ret;
  }
};

class DummyGroundXSettingsComponent:public XSettingsComponent{
 public:
  std::vector<Setting> get_all_settings() override{
    std::vector<openhd::Setting> ret={
        openhd::Setting{"GROUND_X",openhd::IntSetting{10, nullptr}},
        openhd::Setting{"GROUND_Y",openhd::IntSetting{1, nullptr}},
        /*openhd::Setting{"SOME_INT",0},
        openhd::Setting{"SOME_FLOAT",0.0f},
        openhd::Setting{"SOME_STRING",std::string("hello")}*/
    };
    return ret;
  }
};
}


}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_XSETTINGSCOMPONENT_H_
