//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_WIFICARDSCOMMANDSHELPER_H
#define OPENHD_WIFICARDSCOMMANDSHELPER_H

#include "openhd-log.hpp"
#include "OHDWifiCard.hpp"
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
  ss<<"WifiCards::set_txpower("<<txpower_milli_watt<<" mW | "<<tx_power_milli_dbm<<" milli dBm)"<<" for " << card.interface_name << "\n";
  std::cout<<ss.str();
  std::vector<std::string> args{"dev", card.interface_name, "set", "txpower", "fixed", std::to_string(tx_power_milli_dbm)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}
// Not sure which one is better
// https://linux.die.net/man/8/iwconfig
// https://askubuntu.com/questions/597546/iwconfig-wlan0-txpower-30mw-not-working
static bool set_txpower2(const WiFiCard& card,const uint32_t txpower_milli_watt){
  std::stringstream ss;
  ss<<"WifiCards::set_txpower2("<<txpower_milli_watt<<" mW)"<<" for " << card.interface_name << "\n";
  std::cout<<ss.str();
  std::vector<std::string> args{ card.interface_name, "txpower", (std::to_string(txpower_milli_watt)+"mW")};
  bool success = OHDUtil::run_command("iwconfig", args);
  return success;
}

static bool enable_monitor_mode(const WiFiCard &card) {
  std::cout << "WifiCards::enable_monitor_mode(" << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

// https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/configuring_and_managing_networking/configuring-networkmanager-to-ignore-certain-devices_configuring-and-managing-networking
// example: nmcli device set wlx244bfeb71c05 managed no
// blacklist the card from network manager (so we can safely do our own thing, aka wifibroadcast) with it
// NOTE: this is not permament between restarts - but that is exactly what we want,
// since on each restart we might do different things with the wifi card(s)
static bool network_manager_set_card_unmanaged(const WiFiCard &card){
  std::cout<<"Set card "<<card.interface_name<<" to unmanaged by NetworkManager\n";
  bool success = OHDUtil::run_command("nmcli",{"device","set",card.interface_name,"managed","no"});
  return success;
}
}
#endif //OPENHD_WIFICARDSCOMMANDSHELPER_H
