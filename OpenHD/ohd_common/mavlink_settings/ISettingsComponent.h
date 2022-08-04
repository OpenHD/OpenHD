//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_

#include <utility>
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
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
	std::stringstream ss;
	ss<<"Requested change "<<id<<" to "<<std::to_string(requested_value)<<"\n";
	std::cout<<ss.str();
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

class ISettingsComponent{
 public:
  // all the settings this component uses
  virtual std::vector<Setting> get_all_settings()=0;
 public:
  ISettingsComponent()=default;
  // delete copy and move constructor
  ISettingsComponent(const ISettingsComponent&)=delete;
  ISettingsComponent(const ISettingsComponent&&)=delete;
};


static void validate_provided_ids(const std::vector<Setting>& settings){
  // we need to have unique setting string ids. If there is a duplicate, this would be a programmers error,
  // and when used correctly should be found during debugging by this method.
  std::map<std::string,void*> test;
  for(const auto& setting:settings){
	assert(setting.id.length()<=16);
	assert(test.find(setting.id)==test.end());
	test[setting.id]=nullptr;
  }
}

namespace testing {
// For testing
static std::vector<Setting> create_dummy_camera_settings() {
  std::vector<openhd::Setting> ret = {
	  openhd::Setting{"VIDEO_WIDTH", openhd::IntSetting{640, nullptr}},
	  openhd::Setting{"VIDEO_HEIGHT", openhd::IntSetting{480, nullptr}},
	  openhd::Setting{"VIDEO_FPS", openhd::IntSetting{30, nullptr}},
	  openhd::Setting{"VIDEO_FORMAT", openhd::IntSetting{0, nullptr}},
	  openhd::Setting{"V_BITRATE_MBITS", openhd::IntSetting{10, nullptr}},
  };
  return ret;
}
static std::vector<Setting> create_dummy_ground_settings() {
  std::vector<openhd::Setting> ret = {
	  openhd::Setting{"GROUND_X", openhd::IntSetting{10, nullptr}},
	  openhd::Setting{"GROUND_Y", openhd::IntSetting{1, nullptr}},
	  /*openhd::Setting{"SOME_INT",0},
	  openhd::Setting{"SOME_FLOAT",0.0f},
	  openhd::Setting{"SOME_STRING",std::string("hello")}*/
  };
  return ret;
}
}

}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
