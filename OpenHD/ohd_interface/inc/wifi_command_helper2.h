//
// Created by consti10 on 18.11.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_
#define OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_

#include <cstdint>
#include <string>
#include <optional>

// Pretty much taken from https://github.com/webbbn/wifibroadcast_bridge/blob/9220947fd01f6aaf58adc271037b550ce5385b1e/src/raw_socket.cc
// has some advantages but also some disadvantages over using the "run terminal commands" workaround.
namespace wifi::commandhelper2{

bool set_wifi_up_down(const std::string &device, bool up);

bool set_wifi_monitor_mode(const std::string &device);

// set frequency and optionally also the channel width
bool set_wifi_frequency(const std::string &device, uint32_t freq_mhz,std::optional<uint32_t> channel_width=std::nullopt);

// See https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h#L1905
// NOTE: even linux seems to have no idea what mBm means - rtl8812au interprets that not as milli(1000)dBm, but dBm/100
bool set_wifi_txpower(const std::string &device, uint32_t tx_power_mBm);

}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_
