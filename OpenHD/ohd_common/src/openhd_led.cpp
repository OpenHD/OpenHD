//
// Created by consti10 on 01.02.24
// Improved by Rapha on 04.08.24

#include "openhd_led.h"

#include <chrono>
#include <thread>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

namespace openhd::led_util {

void toggle_led(const std::string& filename, const bool on) {
  const auto content = on ? "1" : "0";
  OHDFilesystemUtil::write_file(filename, content);
}

void led_on_off_delayed(const std::string& filename, const std::chrono::milliseconds& delay1, const std::chrono::milliseconds& delay2) {
  toggle_led(filename, false);
  std::this_thread::sleep_for(delay1);
  toggle_led(filename, true);
  std::this_thread::sleep_for(delay2);
}

void blink_leds_fast(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay) {
  led_on_off_delayed(secondary_filename, delay, delay);
  led_on_off_delayed(primary_filename, delay, delay);
}

void blink_leds_slow(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay) {
  led_on_off_delayed(primary_filename, delay, delay);
}

void blink_leds_alternating(const std::string& primary_filename, const std::string& secondary_filename, const std::chrono::milliseconds& delay, std::atomic<bool>& running) {
  while (running) {
    toggle_led(secondary_filename, true);
    toggle_led(primary_filename, false);
    std::this_thread::sleep_for(delay);
    toggle_led(secondary_filename, false);
    toggle_led(primary_filename, true);
    std::this_thread::sleep_for(delay);
  }
}

}  // namespace openhd::led_util

namespace openhd::rpi {

static constexpr auto secondary_led_filename = "/sys/class/leds/PWR/brightness";
static constexpr auto primary_led_filename = "/sys/class/leds/ACT/brightness";

void toggle_secondary_led(const bool on) {
  openhd::led_util::toggle_led(secondary_led_filename, on);
}

void toggle_primary_led(const bool on) {
  openhd::led_util::toggle_led(primary_led_filename, on);
}

void blink_leds_fast(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_fast(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_slow(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_slow(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_alternating(const std::chrono::milliseconds& delay, std::atomic<bool>& running) {
  openhd::led_util::blink_leds_alternating(primary_led_filename, secondary_led_filename, delay, running);
}

}  // namespace openhd::rpi

namespace openhd::zero3w {

static constexpr auto secondary_led_filename = "/sys/class/leds/mmc0::/brightness";
static constexpr auto primary_led_filename = "/sys/class/leds/board-led/brightness";

void toggle_secondary_led(const bool on) {
  openhd::led_util::toggle_led(secondary_led_filename, on);
}

void toggle_primary_led(const bool on) {
  openhd::led_util::toggle_led(primary_led_filename, on);
}

void blink_leds_fast(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_fast(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_slow(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_slow(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_alternating(const std::chrono::milliseconds& delay, std::atomic<bool>& running) {
  openhd::led_util::blink_leds_alternating(primary_led_filename, secondary_led_filename, delay, running);
}

}  // namespace openhd::zero3w

namespace openhd::radxacm3 {

static constexpr auto secondary_led_filename = "/sys/class/leds/pwr-led-red/brightness";
static constexpr auto primary_led_filename = "/sys/class/leds/pi-led-green/brightness";

void toggle_secondary_led(const bool on) {
  openhd::led_util::toggle_led(secondary_led_filename, on);
}

void toggle_primary_led(const bool on) {
  openhd::led_util::toggle_led(primary_led_filename, on);
}

void blink_leds_fast(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_fast(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_slow(const std::chrono::milliseconds& delay) {
  openhd::led_util::blink_leds_slow(primary_led_filename, secondary_led_filename, delay);
}

void blink_leds_alternating(const std::chrono::milliseconds& delay, std::atomic<bool>& running) {
  openhd::led_util::blink_leds_alternating(primary_led_filename, secondary_led_filename, delay, running);
}

}  // namespace openhd::radxacm3

openhd::LEDManager& openhd::LEDManager::instance() {
  static LEDManager instance{};
  return instance;
}

void openhd::LEDManager::set_secondary_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_secondary_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::toggle_secondary_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::toggle_secondary_led(on);
  }
}

void openhd::LEDManager::set_primary_led_status(int status) {
  const bool on = status != STATUS_ON;
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::toggle_primary_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::toggle_primary_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::toggle_primary_led(on);
  }
}

openhd::LEDManager::LEDManager() : m_loading_thread(nullptr), m_running(false), m_has_error(false), m_is_loading(false) {}

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
    } else if (m_is_loading) {
      blink_loading();
    } else {
      blink_okay();
    }
  }
}

void openhd::LEDManager::blink_okay() {
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::blink_leds_fast(std::chrono::milliseconds(50));
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::blink_leds_fast(std::chrono::milliseconds(50));
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::blink_leds_fast(std::chrono::milliseconds(50));
  }
}

void openhd::LEDManager::blink_loading() {
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::blink_leds_slow(std::chrono::milliseconds(200));
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::blink_leds_slow(std::chrono::milliseconds(200));
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::blink_leds_slow(std::chrono::milliseconds(200));
  }
}

void openhd::LEDManager::blink_error() {
  if (OHDPlatform::instance().is_rpi()) {
    openhd::rpi::blink_leds_alternating(std::chrono::milliseconds(50), m_running);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::zero3w::blink_leds_alternating(std::chrono::milliseconds(50), m_running);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::radxacm3::blink_leds_alternating(std::chrono::milliseconds(50), m_running);
  }
}

void openhd::LEDManager::set_status_okay() {
  if (m_is_loading) {
    stop_loading_thread();
  }
  m_has_error = false;
  m_is_loading = false;
  start_loading_thread();
}

void openhd::LEDManager::set_status_loading() {
  if (m_has_error) {
    stop_loading_thread();
  }
  m_has_error = false;
  m_is_loading = true;
  start_loading_thread();
}

void openhd::LEDManager::set_status_error() {
  if (m_is_loading) {
    stop_loading_thread();
  }
  m_has_error = true;
  m_is_loading = false;
  start_loading_thread();
}

void openhd::LEDManager::set_status_stopped() {
  stop_loading_thread();
  // it's weird but it's inverted
  set_primary_led_status(STATUS_ON);
  set_secondary_led_status(STATUS_ON);
}
