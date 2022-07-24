//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_WIFICARDSCOMMANDSHELPER_H
#define OPENHD_WIFICARDSCOMMANDSHELPER_H

#include "openhd-log.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"
#include "validate_settings_helper.h"

// Helper for the commands we need to do stuff with a wifi card (like setting the frequency, txpower, monitor mode, ...)
// In general, just a wrapper around the corresponding system commands.

namespace WifiCardCommandHelper{

static bool set_card_state(const WiFiCard &card, bool up) {
  std::cout << "WifiCards::set_card_state(" << up << ") for " << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"link", "set", "dev", card.interface_name, up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

static bool set_frequency(const WiFiCard &card, const uint32_t frequency) {
  std::cout << "WifiCards::set_frequency(" << frequency << ") for " << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "freq", std::to_string(frequency)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

static bool set_frequency_and_channel_width(const WiFiCard &card, const uint32_t frequency,bool width_40) {
  std::cout << "WifiCards::set_frequency(" << frequency << ") for " << card.interface_name << ")" << std::endl;
  const std::string channel_width=width_40 ? "HT40+" : "HT20";
  std::vector<std::string> args{"dev", card.interface_name, "set", "freq", std::to_string(frequency), channel_width};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

// Consti10: this at least changes what then iw dev displays. If it internally has an effect is not yet tested.
// I think txpower is in milli dbm
static bool set_txpower(const WiFiCard &card, const uint32_t txpower_milli_watt) {
  auto tx_power_milli_dbm=openhd::milli_watt_to_milli_dbm(txpower_milli_watt);
  std::stringstream ss;
  ss<<"WifiCards::set_txpower("<<txpower_milli_watt<<" mW | "<<tx_power_milli_dbm<<" milli dBm\n";
  std::cout<<ss.str();
  std::vector<std::string> args{"dev", card.interface_name, "set", "txpower", "fixed", std::to_string(tx_power_milli_dbm)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

static bool enable_monitor_mode(const WiFiCard &card) {
  std::cout << "WifiCards::enable_monitor_mode(" << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}
}
#endif //OPENHD_WIFICARDSCOMMANDSHELPER_H
