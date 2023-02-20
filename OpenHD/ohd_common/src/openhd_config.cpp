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
    inih::INIReader r{"/home/consti10/Desktop/OpenHD/OpenHD/ohd_common/config/hardware.config"};
    // Get and parse the ini value
    ret.WIFI_ENABLE_AUTODETECT = r.Get<bool>("wifi", "WIFI_ENABLE_AUTODETECT");
    ret.WIFI_WB_LINK_CARDS = r.GetVector<std::string>("wifi", "WIFI_WB_LINK_CARDS");
    ret.WIFI_WIFI_HOTSPOT_CARD = r.Get<std::string>("wifi", "WIFI_WIFI_HOTSPOT_CARD");

    ret.CAMERA_ENABLE_AUTODETECT = r.Get<bool>("camera", "CAMERA_ENABLE_AUTODETECT");
    ret.CAMERA_N_CAMERAS = r.Get<int>("camera", "CAMERA_N_CAMERAS");
    ret.CAMERA_CAMERA0_TYPE = r.Get<std::string>("camera", "CAMERA_CAMERA0_TYPE");
    ret.CAMERA_CAMERA1_TYPE = r.Get<std::string>("camera", "CAMERA_CAMERA1_TYPE");

    return ret;
  }catch (std::exception& exception){
    get_logger()->error("Ill-formatted config file {}",exception.what());
  }
  return {};
}

void openhd::persist_config(const openhd::Config& config) {

}

void openhd::debug_config(const openhd::Config& config) {
  // TODO
}
