//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_

#include <cassert>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "openhd_spdlog.h"

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
	openhd::log::get_default()->debug("Requested change {} to {}",id,requested_value);
	return true;
  };
  //std::function<T()> get_callback= nullptr;
};
using IntSetting=SettingImpl<int>;
using FloatSetting=SettingImpl<float>;
using StringSetting=SettingImpl<std::string>;

struct Setting{
  // Do not mutate me
  std::string id;
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

static bool validate_yes_or_no(int value){
  return value==0 || value==1;
}

// Helper for creating read-only params- they can be usefully for debugging
static Setting create_read_only_int(const std::string& id,int value){
  auto cb=[](const std::string&,int){
    return false;
  };
  return Setting{id, openhd::IntSetting{value, cb}};
}

// Creates a read - only parameter - we repurpose the mavlink param set for reliably showing more info to
// the user / developer. Can be quite nice for debugging.
// Since the n of characters are limited, this might cut away parts of value
static Setting create_read_only_string(const std::string& id,std::string value){
  if(value.length()>15){
     value.resize(15);
  }
  auto cb=[](const std::string&,const std::string&){
    return false;
  };
  return Setting{id, openhd::StringSetting {value, cb}};
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
// A size of 0 creates issues with the param server, but it is possible we don't have any params if
// none were addable during run time due
static void append_dummy_if_empty(std::vector<Setting>& ret){
  if(ret.empty()){
    auto tmp=openhd::IntSetting{0};
    const std::string id="DUMMY";
    ret.push_back(Setting{id,tmp});
  }
}
}

}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
