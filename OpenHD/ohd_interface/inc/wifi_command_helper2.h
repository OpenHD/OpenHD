//
// Created by consti10 on 18.11.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_
#define OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_

#include <cstdint>
#include <string>

// Pretty much taken from https://github.com/webbbn/wifibroadcast_bridge/blob/9220947fd01f6aaf58adc271037b550ce5385b1e/src/raw_socket.cc
namespace wifi::commandhelper2{

static bool set_wifi_up_down(const std::string &device, bool up);

static bool set_wifi_monitor_mode(const std::string &device);

static bool set_wifi_frequency(const std::string &device, uint32_t freq_mhz);

static bool set_wifi_txpower(const std::string &device, uint32_t power_mbm);

}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFI_COMMAND_HELPER2_H_
