/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "openhd_led.h"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// Cache for checking file existence
static std::unordered_map<std::string, bool> led_file_cache;

// Initialize file cache
static void initialize_led_file_cache() {
  if (OHDPlatform::instance().is_rpi()) {
    led_file_cache["/sys/class/leds/PWR/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/PWR/brightness");
    led_file_cache["/sys/class/leds/ACT/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/ACT/brightness");
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    led_file_cache["/sys/class/leds/pwr-led-red/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/pwr-led-red/brightness");
    led_file_cache["/sys/class/leds/pi-led-green/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/pi-led-green/brightness");
  } else if (OHDPlatform::instance().is_rock5_a_b()) {
    led_file_cache["/sys/class/leds/user-led2/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/user-led2/brightness");
  } else if (OHDPlatform::instance().is_rock5_a()) {
    led_file_cache["/sys/class/leds/user-led1/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/user-led1/brightness");
  } else if (OHDPlatform::instance().is_rock5_b()) {
    led_file_cache["/sys/class/leds/mmc0::/brightness"] =
        OHDFilesystemUtil::exists("/sys/class/leds/mmc0::/brightness");
  } else if (OHDPlatform::instance().is_x20()) {
    led_file_cache["/sys/class/leds/openhd-x20dev:red:usr/brightness"] =
        OHDFilesystemUtil::exists(
            "/sys/class/leds/openhd-x20dev:red:usr/brightness");
    led_file_cache["/sys/class/leds/openhd-x20dev:green:usr/brightness"] =
        OHDFilesystemUtil::exists(
            "/sys/class/leds/openhd-x20dev:green:usr/brightness");
  }
}

static void toggle_led(const std::string &filename, const bool on) {
  // Check if the file exists using the cache
  if (led_file_cache.find(filename) != led_file_cache.end() &&
      led_file_cache[filename]) {
    const auto content = on ? "1" : "0";
    OHDFilesystemUtil::write_file(filename, content);
  }
}

static void toggle_secondary_led(const bool on) {
  if (OHDPlatform::instance().is_rpi()) {
    toggle_led("/sys/class/leds/PWR/brightness", on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    toggle_led("/sys/class/leds/pwr-led-red/brightness", on);
  } else if (OHDPlatform::instance().is_rock5_a_b()) {
    toggle_led("/sys/class/leds/user-led2/brightness", on);
  } else if (OHDPlatform::instance().is_x20()) {
    toggle_led("/sys/class/leds/openhd-x20dev:red:usr/brightness", on);
  }
}

static void toggle_primary_led(const bool on) {
  if (OHDPlatform::instance().is_rpi()) {
    toggle_led("/sys/class/leds/ACT/brightness", on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    toggle_led("/sys/class/leds/board-led/brightness", on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    toggle_led("/sys/class/leds/pi-led-green/brightness", on);
  } else if (OHDPlatform::instance().is_rock5_a()) {
    toggle_led("/sys/class/leds/user-led1/brightness", on);
  } else if (OHDPlatform::instance().is_rock5_b()) {
    toggle_led("/sys/class/leds/mmc0::/brightness", on);
  } else if (OHDPlatform::instance().is_x20()) {
    toggle_led("/sys/class/leds/openhd-x20dev:green:usr/brightness", on);
  }
}

static void secondary_led_on_off_delayed(
    const std::chrono::milliseconds &delay1,
    const std::chrono::milliseconds &delay2) {
  toggle_secondary_led(false);
  std::this_thread::sleep_for(delay1);
  toggle_secondary_led(true);
  std::this_thread::sleep_for(delay2);
}

static void primary_led_on_off_delayed(
    const std::chrono::milliseconds &delay1,
    const std::chrono::milliseconds &delay2) {
  toggle_primary_led(false);
  toggle_secondary_led(false);
  std::this_thread::sleep_for(delay1);
  toggle_primary_led(true);
  std::this_thread::sleep_for(delay2);
}

static void blink_leds_fast(const std::chrono::milliseconds &delay) {
  secondary_led_on_off_delayed(delay, delay);
  primary_led_on_off_delayed(delay, delay);
}

static void blink_leds_slow(const std::chrono::milliseconds &delay) {
  primary_led_on_off_delayed(delay, delay);
}

static void blink_leds_alternating(const std::chrono::milliseconds &delay,
                                   std::atomic<bool> &running) {
  while (running) {
    toggle_secondary_led(true);
    toggle_primary_led(false);
    std::this_thread::sleep_for(delay);
    toggle_secondary_led(false);
    toggle_primary_led(true);
    std::this_thread::sleep_for(delay);
  }
}

openhd::LEDManager &openhd::LEDManager::instance() {
  static LEDManager instance{};
  return instance;
}

void openhd::LEDManager::set_secondary_led_status(int status) {
  const bool on = status != STATUS_ON;
  toggle_secondary_led(on);
}

void openhd::LEDManager::set_primary_led_status(int status) {
  const bool on = status != STATUS_ON;
  toggle_primary_led(on);
}

openhd::LEDManager::LEDManager()
    : m_loading_thread(nullptr),
      m_running(false),
      m_has_error(false),
      m_is_loading(false) {}

openhd::LEDManager::~LEDManager() { stop_loading_thread(); }

void openhd::LEDManager::start_loading_thread() {
  if (m_running) return;  // already running

  m_running = true;
  m_loading_thread =
      std::make_unique<std::thread>(&LEDManager::loading_loop, this);
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
  blink_leds_fast(std::chrono::milliseconds(50));
}

void openhd::LEDManager::blink_loading() {
  blink_leds_slow(std::chrono::milliseconds(200));
}

void openhd::LEDManager::blink_error() {
  blink_leds_alternating(std::chrono::milliseconds(50), m_running);
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
