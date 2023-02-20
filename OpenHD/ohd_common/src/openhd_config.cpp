//
// Created by consti10 on 17.02.23.
//

#include "openhd_config.h"
#include "openhd_spdlog.h"

#include "../lib/ini/ini.hpp"

static std::shared_ptr<spdlog::logger> get_logger(){
  return openhd::log::create_or_get("config");
}

openhd::Config openhd::load_config() {
  try{
    openhd::Config ret{};
    inih::INIReader r{"/home/consti10/Desktop/config.config"};
    // Get and parse the ini value
    ret.force_dummy_camera = r.Get<bool>("camera", "FORCE_DUMMY_CAMERA");
    ret.force_custom_unmanaged_camera=r.Get<bool>("camera", "FORCE_CUSTOM_UNMANAGED_CAMERA");
    ret.force_ip_camera=r.Get<bool>("camera", "FORCE_IP_CAMERA");
    return ret;
  }catch (std::exception& exception){
    get_logger()->warn("Ill-formatted config file {}",exception.what());
  }
  return {};
}

void openhd::persist_config(const openhd::Config& config) {

}

void openhd::debug_config(const openhd::Config& config) {
  get_logger()->debug("force_dummy_camera:{}",config.force_dummy_camera);
}
