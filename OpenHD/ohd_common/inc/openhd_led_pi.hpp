//
// Created by consti10 on 12.07.22.
//

#ifndef OPENHD_OPENHD_LED_ERROR_CODES_H
#define OPENHD_OPENHD_LED_ERROR_CODES_H

#include <chrono>
#include <thread>
#include <utility>

#include "openhd_platform.hpp"
#include "openhd_spdlog.hpp"
#include "openhd_util.h"
#include "openhd_util_filesystem.hpp"

// NOTE: Some PI's allow toggling both the red and green led
// All pi's allow toggling the red led
namespace openhd::rpi{
// so far, I have only tested this on the RPI 4 and CM4
static void toggle_red_led(const bool on){
  static constexpr auto filename="/sys/class/leds/led1/brightness";
  if(!OHDFilesystemUtil::exists(filename)){
    openhd::log::get_default()->debug("RPI LED1 brightness does not exist\n");
    return;
  }
  const auto content=on ? "1":"0";
  OHDFilesystemUtil::write_file(filename,content);
}
// I think the green led only supports on/off on the 4th generation pis
static void toggle_green_led(const bool on){
  static constexpr auto filename="/sys/class/leds/led0/brightness";
  if(!OHDFilesystemUtil::exists(filename)){
    openhd::log::get_default()->debug("RPI LED0 brightness does not exist");
    return;
  }
  const auto content=on ? "1":"0";
  OHDFilesystemUtil::write_file(filename,content);
}
// toggle red led off, wait for delay, then toggle it on,wait for delay
static void red_led_on_off_delayed(const std::chrono::milliseconds &delay1,const std::chrono::milliseconds &delay2) {
  rpi::toggle_red_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_red_led(true);
  std::this_thread::sleep_for(delay2);
}

// toggle green led off, wait for delay1, then toggle it on, wait for delay2
static void green_led_on_off_delayed(const std::chrono::milliseconds &delay1,const std::chrono::milliseconds &delay2){
  rpi::toggle_green_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_green_led(true);
  std::this_thread::sleep_for(delay2);
}

}

#endif //OPENHD_OPENHD_LED_ERROR_CODES_H
