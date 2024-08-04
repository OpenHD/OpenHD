#include "openhd_led.h"

#include <chrono>
#include <thread>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// NOTE: Some PI's allow toggling both the red and green led
// All pi's allow toggling the red led
namespace openhd::rpi {
// Existing functions ...

static void red_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                   const std::chrono::milliseconds &delay2) {
  rpi::toggle_red_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_red_led(true);
  std::this_thread::sleep_for(delay2);
}

static void green_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                     const std::chrono::milliseconds &delay2) {
  rpi::toggle_green_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_green_led(true);
  std::this_thread::sleep_for(delay2);
}

// Blink both LEDs fast
static void blink_leds_fast(const std::chrono::milliseconds &delay) {
  rpi::red_led_on_off_delayed(delay, delay);
  rpi::green_led_on_off_delayed(delay, delay);
}

// Blink LEDs in alternating fashion
static void blink_leds_alternating(const std::chrono::milliseconds &delay) {
  while (true) {
    rpi::toggle_red_led(true);
    rpi::toggle_green_led(false);
    std::this_thread::sleep_for(delay);
    rpi::toggle_red_led(false);
    rpi::toggle_green_led(true);
    std::this_thread::sleep_for(delay);
  }
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

namespace openhd::radxacm3 {

static void toggle_red_led(const bool on) {
  static constexpr auto filename = "/sys/class/leds/pwr-led-red/brightness";
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}

static void toggle_green_led(const bool on) {
  static constexpr auto filename = "/sys/class/leds/pi-led-green/brightness";
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}

}  // namespace openhd::radxacm3

openhd::LEDManager &openhd::LEDManager::instance() {
  static LEDManager instance{};
  return instance;
}

void openhd::LEDManager::set_red_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::toggle_red_led(on);
  }
}

void openhd::LEDManager::set_green_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_green_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::toggle_green_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::toggle_green_led(on);
  }
}

openhd::LEDManager::LEDManager() : m_loading_thread(nullptr), m_running(false) {}

openhd::LEDManager::~LEDManager() {
  stop_loading_thread();
}

void openhd::LEDManager::start_loading_thread() {
  if (m_running) return; // already running

  m_running = true;
  m_loading_thread = std::make_unique<std::thread>(&LEDManager::loading_loop, this);
}

void openhd::LEDManager::stop_loading_thread() {
  if (m_running) {
    m_running = false;
    if (m_loading_thread && m_loading_thread->joinable()) {
      m_loading_thread->join();
    }
    m_loading_thread = nullptr;
  }
}

void openhd::LEDManager::loading_loop() {
  while (m_running) {
    if (m_has_error) {
      blink_error();
    } else {
      blink_okay();
    }
  }
}

void openhd::LEDManager::blink_okay() {
  // Blink both LEDs fast
  openhd::rpi::blink_leds_fast(std::chrono::milliseconds(50));
}

void openhd::LEDManager::blink_error() {
  // Blink LEDs in alternating fashion
  openhd::rpi::blink_leds_alternating(std::chrono::milliseconds(50));
}

void openhd::LEDManager::set_status_okay() {
  if (m_has_error) {
    set_status_error();
  }
  m_has_error = false;
  start_loading_thread();
}

void openhd::LEDManager::set_status_loading() {
  m_has_error = false;
  start_loading_thread();
}

void openhd::LEDManager::set_status_error() {
  m_has_error = true;
  start_loading_thread();
}


