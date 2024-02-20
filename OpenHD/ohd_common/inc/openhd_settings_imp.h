//
// Created by consti10 on 05.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_

#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace openhd {

// Util - we have a default impl. that 'does nothing' but prints a message
std::function<bool(std::string id, int requested_value)>
create_log_only_cb_int();
std::function<bool(std::string id, std::string requested_value)>
create_log_only_cb_string();

// int / string setting general layout:
// value:
// The value which the ground station (e.g. the user) can modify via mavlink
// after passing the implemented sanity checks (e.g. the value that is changed
// by the mavlink parameter provider when OpenHD returned true in the
// change_callback). change_callback: This callback is called every time the
// user wants to change the parameter (T value) from value x to value y (via
// mavlink) return true to accept the value, otherwise return false. We have a
// default implementation that just prints the change request and always returns
// true, mostly for debugging / testing. But in general, all OpenHD modules that
// are configurable overwrite this callback with their own proper
// implementation. get_callback: Quite dirty - all the params in openhd are
// changed by the user via mavlink only - Except channel frequency and channel
// width during the channel scan feature. Workaround for this rare case - don't
// ask ;)

struct IntSetting {
  int value;
  std::function<bool(std::string id, int requested_value)> change_callback =
      create_log_only_cb_int();
  std::function<int()> get_callback = nullptr;
};
struct StringSetting {
  std::string value;
  std::function<bool(std::string id, std::string requested_value)>
      change_callback = create_log_only_cb_string();
  std::function<std::string()> get_callback = nullptr;
};

struct Setting {
  // Do not mutate me
  std::string id;
  std::variant<IntSetting, StringSetting> setting;
};

// we need to have unique setting string ids. Creating duplicates by accident is
// not uncommon when adding new settings, and when this function is used
// properly we can catch those mistakes at run time.
void validate_provided_ids(const std::vector<Setting>& settings);

static bool validate_yes_or_no(int value) { return value == 0 || value == 1; }

// Helper for creating read-only params- they can be usefully for debugging
Setting create_read_only_int(const std::string& id, int value);

// Creates a read - only parameter - we repurpose the mavlink param set for
// reliably showing more info to the user / developer. Can be quite nice for
// debugging. Since the n of characters are limited, this might cut away parts
// of value
Setting create_read_only_string(const std::string& id, std::string value);

// Helper function - adds a new int param that has an ID, an initial value,
// and a cb that is called when the value shall be changed by mavlink
void append_int_param(std::vector<Setting>& ret, const std::string& ID,
                      int value,
                      const std::function<bool(int requested_value)>& cb);

namespace testing {
std::vector<Setting> create_dummy_camera_settings();
std::vector<Setting> create_dummy_ground_settings();
// A size of 0 creates issues with the param server, but it is possible we don't
// have any params if none were addable during run time due
void append_dummy_if_empty(std::vector<Setting>& ret);
}  // namespace testing

}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_ISETTINGSCOMPONENT_H_
