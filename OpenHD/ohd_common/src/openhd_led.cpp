//
// Created by consti10 on 01.02.24.
//

#include "openhd_led.h"

#include <chrono>
#include <thread>
#include <utility>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// NOTE: Some PI's allow toggling both the red and green led
// All pi's allow toggling the red led
namespace openhd::rpi {
// so far, I have only tested this on the RPI 4 and CM4
static void toggle_red_led(const bool on) {
  // static constexpr auto filename="/sys/class/leds/led1/brightness";
  static constexpr auto filename = "/sys/class/leds/PWR/brightness";
  if (!OHDFilesystemUtil::exists(filename)) {
    openhd::log::get_default()->debug("RPI LED1 brightness does not exist\n");
    return;
  }
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}
// I think the green led only supports on/off on the 4th generation pis
static void toggle_green_led(const bool on) {
  // static constexpr auto filename="/sys/class/leds/led0/brightness";
  static constexpr auto filename = "/sys/class/leds/ACT/brightness";
  if (!OHDFilesystemUtil::exists(filename)) {
    openhd::log::get_default()->debug("RPI LED0 brightness does not exist");
    return;
  }
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}
// toggle red led off, wait for delay, then toggle it on,wait for delay
static void red_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                   const std::chrono::milliseconds &delay2) {
  rpi::toggle_red_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_red_led(true);
  std::this_thread::sleep_for(delay2);
}

// toggle green led off, wait for delay1, then toggle it on, wait for delay2
static void green_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                     const std::chrono::milliseconds &delay2) {
  rpi::toggle_green_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_green_led(true);
  std::this_thread::sleep_for(delay2);
}

}  // namespace openhd::rpi

namespace openhd::zero3w {

static void toggle_red_led(const bool on) {
  static constexpr auto filename = "/sys/class/leds/mmc0::/brightness";
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}

static void toggle_green_led(const bool on) {
  static constexpr auto filename = "/sys/class/leds/board-led/brightness";
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}

}  // namespace openhd::zero3w

openhd::LEDManager &openhd::LEDManager::instance() {
  static LEDManager instance{};
  return instance;
}

void openhd::LEDManager::set_red_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_rock()) {
    openhd::zero3w::toggle_red_led(on);
  }
}

void openhd::LEDManager::set_green_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_green_led(on);
  } else if (OHDPlatform::instance().is_rock()) {
    openhd::zero3w::toggle_green_led(on);
  }
}

openhd::LEDManager::LEDManager() {
  // m_run= true;
  // m_manage_thread = std::make_unique<std::thread>(&LEDManager::loop, this);
}

openhd::LEDManager::~LEDManager() {
  /*m_run= false;
  if(m_manage_thread->joinable()){
    m_manage_thread->join();
  }
  m_manage_thread= nullptr;*/
}

void openhd::LEDManager::loop() {
  /*while (m_run){

  }*/
}
void openhd::LEDManager::set_status_okay() {
  if (m_has_error) {
    set_status_error();
  }
  set_green_led_status(STATUS_ON);
  set_red_led_status(STATUS_OFF);
}
void openhd::LEDManager::set_status_loading() {
  set_green_led_status(STATUS_OFF);
  set_red_led_status(STATUS_OFF);
}
void openhd::LEDManager::set_status_error() {
  set_green_led_status(STATUS_ON);
  set_red_led_status(STATUS_ON);
  m_has_error = true;
}
