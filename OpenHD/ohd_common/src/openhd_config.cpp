//
// Created by consti10 on 17.02.23.
//

#include "openhd_config.h"

#include "../lib/ini/ini.hpp"
#include "config_paths.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static std::string CONFIG_FILE_PATH =
    std::string(getConfigBasePath()) + "hardware.config";

static std::shared_ptr<spdlog::logger> get_logger() {
  return openhd::log::create_or_get("config");
}

void openhd::set_config_file(const std::string& config_file_path) {
  get_logger()->debug("Using custom config file path [{}]", config_file_path);
  CONFIG_FILE_PATH = config_file_path;
}

static openhd::Config load_or_default() {
  try {
    openhd::Config ret{};
    if (!OHDFilesystemUtil::exists(CONFIG_FILE_PATH)) {
      return ret;
    } else {
      get_logger()->warn("Advanced config file [{}] used!", CONFIG_FILE_PATH);
    }
    inih::INIReader r{CONFIG_FILE_PATH};

    // Parse WiFi configuration
    ret.WIFI_ENABLE_AUTODETECT =
        r.Get<bool>("wifi", "WIFI_ENABLE_AUTODETECT", false);
    ret.WIFI_WB_LINK_CARDS =
        r.GetVector<std::string>("wifi", "WIFI_WB_LINK_CARDS");
    ret.WIFI_WIFI_HOTSPOT_CARD =
        r.Get<std::string>("wifi", "WIFI_WIFI_HOTSPOT_CARD", "");
    ret.WIFI_MONITOR_CARD_EMULATE =
        r.Get<bool>("wifi", "WIFI_MONITOR_CARD_EMULATE", false);
    ret.WIFI_FORCE_NO_LINK_BUT_HOTSPOT =
        r.Get<bool>("wifi", "WIFI_FORCE_NO_LINK_BUT_HOTSPOT", false);
    ret.WIFI_LOCAL_NETWORK_ENABLE =
        r.Get<bool>("wifi", "WIFI_LOCAL_NETWORK_ENABLE", false);
    ret.WIFI_LOCAL_NETWORK_SSID =
        r.Get<std::string>("wifi", "WIFI_LOCAL_NETWORK_SSID", "");
    ret.WIFI_LOCAL_NETWORK_PASSWORD =
        r.Get<std::string>("wifi", "WIFI_LOCAL_NETWORK_PASSWORD", "");

    // Parse Network configuration
    ret.NW_ETHERNET_CARD =
        r.Get<std::string>("network", "NW_ETHERNET_CARD", "");
    ret.NW_MANUAL_FORWARDING_IPS =
        r.GetVector<std::string>("network", "NW_MANUAL_FORWARDING_IPS");
    ret.NW_FORWARD_TO_LOCALHOST_58XX =
        r.Get<bool>("network", "NW_FORWARD_TO_LOCALHOST_58XX", false);

    // Parse Ethernet link configuration
    ret.GROUND_UNIT_IP = r.Get<std::string>("ethernet", "GROUND_UNIT_IP", "");
    ret.AIR_UNIT_IP = r.Get<std::string>("ethernet", "AIR_UNIT_IP", "");
    ret.VIDEO_PORT =
        r.Get<int>("ethernet", "VIDEO_PORT", 5000);  // Default port 5000
    ret.TELEMETRY_PORT =
        r.Get<int>("ethernet", "TELEMETRY_PORT", 5600);  // Default port 5600

    // Parse Generic configuration
    ret.GEN_ENABLE_LAST_KNOWN_POSITION =
        r.Get<bool>("generic", "GEN_ENABLE_LAST_KNOWN_POSITION", false);
    ret.GEN_RF_METRICS_LEVEL = r.Get<int>("generic", "GEN_RF_METRICS_LEVEL", 0);
    ret.GEN_NO_QOPENHD_AUTOSTART =
        r.Get<bool>("generic", "GEN_NO_QOPENHD_AUTOSTART", false);

    // Parse Development configuration
    ret.DEV_ENABLE_MICROHARD =
        r.Get<bool>("dev", "DEV_ENABLE_MICROHARD", false);

    return ret;
  } catch (std::exception& exception) {
    get_logger()->error("Ill-formatted config file {}",
                        std::string(exception.what()));
  }
  return {};
}

openhd::Config openhd::load_config() {
  static openhd::Config config = load_or_default();
  return config;
}

void openhd::debug_config(const openhd::Config& config) {
  get_logger()->debug(
      "WIFI_ENABLE_AUTODETECT:{}, WIFI_WB_LINK_CARDS:{}, "
      "WIFI_WIFI_HOTSPOT_CARD:{},WIFI_MONITOR_CARD_EMULATE:{}\n"
      "WIFI_FORCE_NO_LINK_BUT_HOTSPOT:{}, WIFI_LOCAL_NETWORK_ENABLE:{}, "
      "WIFI_LOCAL_NETWORK_SSID:[{}], WIFI_LOCAL_NETWORK_PASSWORD:[{}]\n"
      "NW_MANUAL_FORWARDING_IPS:{},NW_ETHERNET_CARD:{},NW_FORWARD_TO_LOCALHOST_"
      "58XX:{}\n"
      "GEN_RF_METRICS_LEVEL:{}, GEN_NO_QOPENHD_AUTOSTART:{}\n"
      "GROUND_UNIT_IP:{}, AIR_UNIT_IP:{}, VIDEO_PORT:{}, TELEMETRY_PORT:{}\n",
      config.WIFI_ENABLE_AUTODETECT,
      OHDUtil::str_vec_as_string(config.WIFI_WB_LINK_CARDS),
      config.WIFI_WIFI_HOTSPOT_CARD, config.WIFI_MONITOR_CARD_EMULATE,
      config.WIFI_FORCE_NO_LINK_BUT_HOTSPOT, config.WIFI_LOCAL_NETWORK_ENABLE,
      config.WIFI_LOCAL_NETWORK_SSID, config.WIFI_LOCAL_NETWORK_PASSWORD,
      OHDUtil::str_vec_as_string(config.NW_MANUAL_FORWARDING_IPS),
      config.NW_ETHERNET_CARD, config.NW_FORWARD_TO_LOCALHOST_58XX,
      config.GEN_RF_METRICS_LEVEL, config.GEN_NO_QOPENHD_AUTOSTART,
      config.GROUND_UNIT_IP, config.AIR_UNIT_IP, config.VIDEO_PORT,
      config.TELEMETRY_PORT);
}

void openhd::debug_config() {
  auto config = load_config();
  debug_config(config);
}

bool openhd::nw_ethernet_card_manual_active(const openhd::Config& config) {
  if (OHDUtil::contains(config.NW_ETHERNET_CARD, RPI_ETHERNET_ONLY)) {
    return false;
  }
  return true;
}
