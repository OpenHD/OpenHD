//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_WIFICARDSCOMMANDSHELPER_H
#define OPENHD_WIFICARDSCOMMANDSHELPER_H

#include "openhd-util.hpp"
#include "validate_settings_helper.h"
#include "wifi_card.hpp"

// Helper for the commands we need to do stuff with a wifi card (like setting the frequency, txpower, monitor mode, ...)
// In general, just a wrapper around the corresponding system commands.

namespace WifiCardCommandHelper{

static bool set_card_state(const WiFiCard &card, bool up) {
  openhd::log::get_default()->info("WifiCards::set_card_state up "+OHDUtil::yes_or_no(up)+" for "+card.interface_name+")");
  std::vector<std::string> args{"link", "set", "dev", card.interface_name, up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

static bool set_frequency(const WiFiCard &card, const uint32_t frequency) {
  openhd::log::get_default()->info("WifiCards::set_frequency{} for {}",frequency,card.interface_name.c_str());
  std::vector<std::string> args{"dev", card.interface_name, "set", "freq", std::to_string(frequency)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

static bool set_frequency_and_channel_width(const WiFiCard &card, const uint32_t frequency,bool width_40) {
  openhd::log::get_default()->info("WifiCards::set_frequency{} for {} with channel width 40:"+OHDUtil::yes_or_no(width_40),frequency,card.interface_name.c_str());
  const std::string channel_width=width_40 ? "HT40+" : "HT20";
  std::vector<std::string> args{"dev", card.interface_name, "set", "freq", std::to_string(frequency), channel_width};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

// See https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h#L1905
// NOTE: even linux seems to have no idea what mBm means - rtl8812au interprets that not as milli(1000)dBm, but dBm/100
// from iw documentation:
// dev <devname> set txpower <auto|fixed|limit> [<tx power in mBm>]
//		Specify transmit power level and setting type.
static bool iw_set_tx_power_mBm(const WiFiCard &card, const uint32_t tx_power_mBm) {
  std::stringstream ss;
  ss<<"WifiCards::set_txpower("<<tx_power_mBm<<" mBm)"<<" for " << card.interface_name;
  openhd::log::get_default()->info(ss.str());
  std::vector<std::string> args{"dev", card.interface_name, "set", "txpower", "fixed", std::to_string(tx_power_mBm)};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

// Not sure which one is better
// https://linux.die.net/man/8/iwconfig
// https://askubuntu.com/questions/597546/iwconfig-wlan0-txpower-30mw-not-working
static bool iwconfig_set_txpower(const WiFiCard& card,const uint32_t txpower_milli_watt){
  std::stringstream ss;
  ss<<"WifiCards::set_txpower2("<<txpower_milli_watt<<" mW)"<<" for " << card.interface_name;
  openhd::log::get_default()->info(ss.str());
  std::vector<std::string> args{ card.interface_name, "txpower", (std::to_string(txpower_milli_watt)+"mW")};
  bool success = OHDUtil::run_command("iwconfig", args);
  return success;
}

// from iw documentation:
// dev <devname> set monitor <flag>*
//		Set monitor flags. Valid flags are:
//		none:     no special flags
//		fcsfail:  show frames with FCS errors
//		control:  show control frames
//		otherbss: show frames from other BSSes
//		cook:     use cooked mode
//		active:   use active mode (ACK incoming unicast packets)
//		mumimo-groupid <GROUP_ID>: use MUMIMO according to a group id
//		mumimo-follow-mac <MAC_ADDRESS>: use MUMIMO according to a MAC address
static bool enable_monitor_mode(const WiFiCard &card) {
  openhd::log::get_default()->info("WifiCards::enable_monitor_mode("+card.interface_name+")");
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
  openhd::log::get_default()->info("Set card "+card.interface_name+" to unmanaged by NetworkManager");
  bool success = OHDUtil::run_command("nmcli",{"device","set",card.interface_name,"managed","no"});
  return success;
}
}
#endif //OPENHD_WIFICARDSCOMMANDSHELPER_H
