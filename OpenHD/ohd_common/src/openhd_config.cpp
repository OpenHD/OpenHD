//
// Created by consti10 on 17.02.23.
//

#include "openhd_config.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

#include "../lib/ini/ini.hpp"

static std::shared_ptr<spdlog::logger> get_logger(){
  return openhd::log::create_or_get("config");
}

openhd::Config openhd::load_config() {
  try{
    openhd::Config ret{};
    // RPI - allow user(s) to edit the file on the SD card. Otherwise, we still need to figure out what to do
    std::string filename="/home/consti10/Desktop/OpenHD/OpenHD/ohd_common/config/hardware.config";
    const std::string image_filename="/boot/openhd/hardware.config";
    if(OHDFilesystemUtil::exists(image_filename)){
      filename=image_filename;
    }
    inih::INIReader r{filename};
    // Get and parse the ini value
    ret.WIFI_ENABLE_AUTODETECT = r.Get<bool>("wifi", "WIFI_ENABLE_AUTODETECT");
    ret.WIFI_WB_LINK_CARDS = r.GetVector<std::string>("wifi", "WIFI_WB_LINK_CARDS");
    ret.WIFI_WIFI_HOTSPOT_CARD = r.Get<std::string>("wifi", "WIFI_WIFI_HOTSPOT_CARD");

    ret.CAMERA_ENABLE_AUTODETECT = r.Get<bool>("camera", "CAMERA_ENABLE_AUTODETECT");
    ret.CAMERA_N_CAMERAS = r.Get<int>("camera", "CAMERA_N_CAMERAS");
    ret.CAMERA_CAMERA0_TYPE = r.Get<std::string>("camera", "CAMERA_CAMERA0_TYPE");
    ret.CAMERA_CAMERA1_TYPE = r.Get<std::string>("camera", "CAMERA_CAMERA1_TYPE");

	ret.NW_ETHERNET_CARD = r.Get<std::string>("network", "NW_ETHERNET_CARD");
    ret.NW_MANUAL_FORWARDING_IPS =  r.GetVector<std::string>("network", "NW_MANUAL_FORWARDING_IPS");
    return ret;
  }catch (std::exception& exception){
    get_logger()->error("Ill-formatted config file {}",std::string(exception.what()));
  }
  return {};
}

void openhd::debug_config(const openhd::Config& config) {
  get_logger()->debug("WIFI_ENABLE_AUTODETECT:{}, WIFI_WB_LINK_CARDS:{}, WIFI_WIFI_HOTSPOT_CARD:{},\n"
      "CAMERA_ENABLE_AUTODETECT:{}, CAMERA_N_CAMERAS:{}, CAMERA_CAMERA0_TYPE:{}, CAMERA_CAMERA1_TYPE:{}\n"
      "NW_MANUAL_FORWARDING_IPS:{},NW_ETHERNET_CARD:{}",
      config.WIFI_ENABLE_AUTODETECT,OHDUtil::str_vec_as_string(config.WIFI_WB_LINK_CARDS),config.WIFI_WIFI_HOTSPOT_CARD,
      config.CAMERA_ENABLE_AUTODETECT,config.CAMERA_N_CAMERAS,config.CAMERA_CAMERA0_TYPE,config.CAMERA_CAMERA1_TYPE,
      OHDUtil::str_vec_as_string(config.NW_MANUAL_FORWARDING_IPS),config.NW_ETHERNET_CARD
      );
}
void openhd::debug_config() {
  //auto config=load_config();
  //debug_config(config);
}

bool openhd::nw_ethernet_card_manual_active(const openhd::Config &config) {
  if(OHDUtil::contains(config.NW_ETHERNET_CARD,RPI_ETHERNET_ONLY)){
	return false;
  }
  return true;
}
