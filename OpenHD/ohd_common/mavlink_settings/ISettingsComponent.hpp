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

#include "../openhd-spdlog.hpp"

namespace openhd{

// These extra settings implementations exist to avoid a direct dependency on mavlink on any modules that are configurable.
// They are using templates to be type safe, e.g. we let the c++ compiler make sure for us the following cannot happen (example):
// A module creates an int settings, but the callback is called with a float or similar. Or in other words:
// If you have an (int, float, string) setting you do not need to perform any type checking in the callback.
template<class T>
struct SettingImpl{
  // The value which the ground station (e.g. the user) can modify via mavlink after passing the implemented sanity checks
  // (e.g. the value that is changed by the mavlink parameter provider when OpenHD returned true in the change_callback).
  T value;
  // This callback is called every time the user wants to change the parameter (T value) from value x to value y (via mavlink)
  // return true to accept the value, otherwise return false.
  // We have a default implementation that just prints the change request and always returns true, mostly for debugging / testing.
  // But in general, all OpenHD modules that are configurable overwrite this callback with their own proper implementation.
  std::function<bool(std::string id,T requested_value)> change_callback=[](std::string id,T requested_value){
	std::stringstream ss;
	ss<<"Requested change "<<id<<" to "<<requested_value;
	openhd::loggers::get_default()->debug(ss.str());
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

// we need to have unique setting string ids. Creating duplicates by accident is not uncommon when adding new settings, and when
// this function is used properly we can catch those mistakes at run time.
static void validate_provided_ids(const std::vector<Setting>& settings){
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
	  openhd::Setting{"VIDEO_CODEC", openhd::IntSetting{0, nullptr}},
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

// takes a reference because openhd::Setting has no move/copy
static void append_dummy_int_and_string(std::vector<Setting>& ret){
  for(int i=0;i<5;i++){
	auto tmp=openhd::IntSetting{i};
	const std::string id="TEST_INT_"+std::to_string(i);
	ret.push_back(Setting{id,tmp});
  }
  for(int i=0;i<2;i++){
	const std::string value="val"+std::to_string(i);
	auto tmp=openhd::StringSetting{value};
	const std::string id="TEST_STRING_"+std::to_string(i);
	ret.push_back(Setting{id,tmp});
  }
}
}

}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
