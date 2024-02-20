//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_

#include <functional>
#include <string>
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
  // Quite dirty - all the params in openhd are changed by the user via mavlink only -
  // Except channel frequency and channel width during the channel scan feature.
  // Workaround for this rare case - don't ask ;)
  std::function<T()> get_callback= nullptr;
};
using IntSetting=SettingImpl<int>;
using StringSetting=SettingImpl<std::string>;

struct Setting{
  // Do not mutate me
  std::string id;
  std::variant<IntSetting,StringSetting> setting;
};

// we need to have unique setting string ids. Creating duplicates by accident is not uncommon when adding new settings, and when
// this function is used properly we can catch those mistakes at run time.
void validate_provided_ids(const std::vector<Setting>& settings);

static bool validate_yes_or_no(int value){
  return value==0 || value==1;
}

// Helper for creating read-only params- they can be usefully for debugging
Setting create_read_only_int(const std::string& id,int value);

// Creates a read - only parameter - we repurpose the mavlink param set for reliably showing more info to
// the user / developer. Can be quite nice for debugging.
// Since the n of characters are limited, this might cut away parts of value
Setting create_read_only_string(const std::string& id,std::string value);

// Helper function - adds a new int param that has an ID, an initial value,
// and a cb that is called when the value shall be changed by mavlink
void append_int_param(std::vector<Setting>& ret,const std::string& ID,int value,
                             const std::function<bool(int requested_value)>& cb);

namespace testing {
std::vector<Setting> create_dummy_camera_settings();
std::vector<Setting> create_dummy_ground_settings();
// A size of 0 creates issues with the param server, but it is possible we don't have any params if
// none were addable during run time due
void append_dummy_if_empty(std::vector<Setting>& ret);
}

}
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
