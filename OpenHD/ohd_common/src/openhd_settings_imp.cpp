//
// Created by consti10 on 20.02.24.
//

#include "openhd_settings_imp.h"

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

#include "openhd_spdlog.h"

std::vector<openhd::Setting> openhd::testing::create_dummy_camera_settings() {
  std::vector<openhd::Setting> ret = {
      openhd::Setting{"VIDEO_WIDTH", openhd::IntSetting{640, nullptr}},
      openhd::Setting{"VIDEO_HEIGHT", openhd::IntSetting{480, nullptr}},
      openhd::Setting{"VIDEO_FPS", openhd::IntSetting{30, nullptr}},
      openhd::Setting{"VIDEO_CODEC", openhd::IntSetting{0, nullptr}},
      openhd::Setting{"V_BITRATE_MBITS", openhd::IntSetting{10, nullptr}},
  };
  return ret;
}

std::vector<openhd::Setting> openhd::testing::create_dummy_ground_settings() {
  std::vector<openhd::Setting> ret = {
      openhd::Setting{"GROUND_X", openhd::IntSetting{10, nullptr}},
      openhd::Setting{"GROUND_Y", openhd::IntSetting{1, nullptr}},
      /*openhd::Setting{"SOME_INT",0},
      openhd::Setting{"SOME_FLOAT",0.0f},
      openhd::Setting{"SOME_STRING",std::string("hello")}*/
  };
  return ret;
}

void openhd::testing::append_dummy_if_empty(std::vector<Setting>& ret) {
  if (ret.empty()) {
    auto tmp = openhd::IntSetting{0};
    const std::string id = "DUMMY";
    ret.push_back(Setting{id, tmp});
  }
}
void openhd::append_int_param(std::vector<Setting>& ret, const std::string& ID,
                              int value, const std::function<bool(int)>& cb) {
  auto cb2 = [cb](std::string param_id,
                  int value) {  // NOLINT(*-unnecessary-value-param)
    const auto before = std::chrono::steady_clock::now();
    const bool ret = cb(value);
    const auto delta = std::chrono::steady_clock::now() - before;
    if (delta > std::chrono::seconds(1)) {
      const auto duration_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
      std::stringstream ss;
      ss << "Changing " << param_id << " to " << value << " took "
         << duration_ms << "ms";
      std::cerr << ss.str() << std::endl;
    }
    return ret;
  };
  ret.push_back(Setting{ID, openhd::IntSetting{value, cb2}});
}

openhd::Setting openhd::create_read_only_string(const std::string& id,
                                                std::string value) {
  if (value.length() > 15) {
    value.resize(15);
  }
  auto cb = [](const std::string&, const std::string&) { return false; };
  return Setting{id, openhd::StringSetting{value, cb}};
}

openhd::Setting openhd::create_read_only_int(const std::string& id, int value) {
  auto cb = [](const std::string&, int) { return false; };
  return Setting{id, openhd::IntSetting{value, cb}};
}

void openhd::validate_provided_ids(const std::vector<Setting>& settings) {
  std::map<std::string, void*> test;
  for (const auto& setting : settings) {
    assert(setting.id.length() <= 16);
    assert(test.find(setting.id) == test.end());
    test[setting.id] = nullptr;
  }
}

std::function<bool(std::string id, int requested_value)>
openhd::create_log_only_cb_int() {
  auto cb = [](std::string id, int value) {
    openhd::log::get_default()->debug("MAVLINK wants to change {} to {}", id,
                                      value);
    return true;
  };
  return cb;
}

std::function<bool(std::string id, std::string requested_value)>
openhd::create_log_only_cb_string() {
  auto cb = [](std::string id, std::string value) {
    openhd::log::get_default()->debug("MAVLINK wants to change {} to {}", id,
                                      value);
    return true;
  };
  return cb;
}
