//
// Created by consti10 on 13.07.22.
//

#include "wifi_command_helper.h"

#include "openhd-util.hpp"

static std::shared_ptr<spdlog::logger> get_logger(){
  return openhd::log::create_or_get("w_helper");
}

bool wifi::commandhelper::rfkill_unblock_all() {
  get_logger()->info("rfkill_unblock_all");
  std::vector<std::string> args{"unblock","all"};
  bool success=OHDUtil::run_command("rfkill",args);
  return success;
}

bool wifi::commandhelper::ip_link_set_card_state(const std::string &device,bool up) {
  get_logger()->info("ip_link_set_card_state {} up {}",device,up);
  std::vector<std::string> args{"link", "set", "dev",device, up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

bool wifi::commandhelper::iw_enable_monitor_mode(const std::string &device) {
  get_logger()->info("iw_enable_monitor_mode {}",device);
  std::vector<std::string> args{"dev", device, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

bool wifi::commandhelper::iw_set_frequency_and_channel_width(const std::string &device, uint32_t freq_mhz, bool width_40) {
  get_logger()->info("iw_set_frequency_and_channel_width {} {}Mhz width{}",device,freq_mhz,width_40);
  const std::string channel_width=width_40 ? "HT40+" : "HT20";
  std::vector<std::string> args{"dev", device, "set", "freq", std::to_string(freq_mhz), channel_width};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

bool wifi::commandhelper::iw_set_tx_power(const std::string &device,uint32_t tx_power_mBm) {
  get_logger()->info("iw_set_tx_power {} {} mBm",device,tx_power_mBm);
  std::vector<std::string> args{"dev",device, "set", "txpower", "fixed", std::to_string(tx_power_mBm)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

bool wifi::commandhelper::nmcli_set_device_unmanaged(const std::string &device) {
  get_logger()->info("nmcli_set_device_unmanaged {}",device);
  bool success = OHDUtil::run_command("nmcli",{"device","set",device,"managed","no"});
  return success;
}

