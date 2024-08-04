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

std::vector<std::string> listLedFoldersWithBrightness(const std::string& baseDir) {
    std::vector<std::string> ledFolders;

    // Get all entries in the base directory
    std::vector<std::string> entries = OHDFilesystemUtil::getAllEntriesFullPathInDirectory(baseDir);

    // Iterate through each entry and check for the brightness file
    for (const std::string& entry : entries) {
        std::string brightnessFile = entry + "/brightness";
        if (OHDFilesystemUtil::exists(brightnessFile)) {
            // Extract the folder name from the full path
            std::string folderName = entry.substr(baseDir.length());
            if (folderName.front() == '/')
                folderName = folderName.substr(1);
            ledFolders.push_back(folderName);
        }
    }

    return ledFolders;
}

void turnOffAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir) {
    for (const std::string& folder : ledFolders) {
        std::string brightnessFile = baseDir + folder + "/brightness";
        openhd::log::get_default()->warn("Turning off LED in folder: {}", folder);
        OHDFilesystemUtil::write_file(brightnessFile, "0");
    }
}

void turnOnAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir) {
    for (const std::string& folder : ledFolders) {
        std::string brightnessFile = baseDir + folder + "/brightness";
        openhd::log::get_default()->warn("Turning on LED in folder: {}", folder);
        OHDFilesystemUtil::write_file(brightnessFile, "1");
    }
}

// NOTE: Some PI's allow toggling both the red and green LED
// All PI's allow toggling the red LED
namespace openhd::rpi {
// so far, I have only tested this on the RPI 4 and CM4
static void toggle_red_led(const bool on) {
  openhd::log::get_default()->warn("toggle_red_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/PWR/brightness";
  if (!OHDFilesystemUtil::exists(filename)) {
    openhd::log::get_default()->warn("RPI LED1 brightness does not exist\n");
    return;
  }
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

// I think the green LED only supports on/off on the 4th generation PI's
static void toggle_green_led(const bool on) {
  openhd::log::get_default()->warn("toggle_green_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/ACT/brightness";
  if (!OHDFilesystemUtil::exists(filename)) {
    openhd::log::get_default()->warn("RPI LED0 brightness does not exist");
    return;
  }
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

// toggle red LED off, wait for delay, then toggle it on, wait for delay
static void red_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                   const std::chrono::milliseconds &delay2) {
  openhd::log::get_default()->warn("red_led_on_off_delayed: delay1={}ms, delay2={}ms", delay1.count(), delay2.count());
  rpi::toggle_red_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_red_led(true);
  std::this_thread::sleep_for(delay2);
}

// toggle green LED off, wait for delay1, then toggle it on, wait for delay2
static void green_led_on_off_delayed(const std::chrono::milliseconds &delay1,
                                     const std::chrono::milliseconds &delay2) {
  openhd::log::get_default()->warn("green_led_on_off_delayed: delay1={}ms, delay2={}ms", delay1.count(), delay2.count());
  rpi::toggle_green_led(false);
  std::this_thread::sleep_for(delay1);
  rpi::toggle_green_led(true);
  std::this_thread::sleep_for(delay2);
}

}  // namespace openhd::rpi

namespace openhd::zero3w {

static void toggle_red_led(const bool on) {
  openhd::log::get_default()->warn("zero3w::toggle_red_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/mmc0::/brightness";
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

static void toggle_green_led(const bool on) {
  openhd::log::get_default()->warn("zero3w::toggle_green_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/board-led/brightness";
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

}  // namespace openhd::zero3w

namespace openhd::radxacm3 {

static void toggle_red_led(const bool on) {
  openhd::log::get_default()->warn("radxacm3::toggle_red_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/pwr-led-red/brightness";
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

static void toggle_green_led(const bool on) {
  openhd::log::get_default()->warn("radxacm3::toggle_green_led: on={}", on);
  static constexpr auto filename = "/sys/class/leds/pi-led-green/brightness";
  const auto content = on ? "1" : "0";
  openhd::log::get_default()->warn("Writing to {}: {}", filename, content);
  OHDFilesystemUtil::write_file(filename, content);
}

}  // namespace openhd::radxacm3

openhd::LEDManager &openhd::LEDManager::instance() {
  static LEDManager instance{};
  return instance;
}

void openhd::LEDManager::set_aux_led_status(int status) {
  openhd::log::get_default()->warn("LEDManager::set_aux_led_status: status={}", status);
  const bool on = status != STATUS_ON;
  openhd::log::get_default()->warn("LEDManager::set_aux_led_status: on={}", on);
  if (OHDPlatform::instance().is_rpi()) {
    openhd::log::get_default()->warn("LEDManager::set_aux_led_status: Platform is RPI");
    openhd::rpi::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::log::get_default()->warn("LEDManager::set_aux_led_status: Platform is Zero3W");
    openhd::zero3w::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::log::get_default()->warn("LEDManager::set_aux_led_status: Platform is Radxa CM3");
    openhd::radxacm3::toggle_red_led(on);
  }
}

void openhd::LEDManager::set_rgb_led_status(int status, int color) {
  openhd::log::get_default()->warn("LEDManager::set_rgb_led_status: status={}", status);
  const bool on = status != STATUS_ON;
  openhd::log::get_default()->warn("LEDManager::set_rgb_led_status: on={}", on);
  if (OHDPlatform::instance().is_rpi()) {
    openhd::log::get_default()->warn("LEDManager::set_rgb_led_status: Platform is RPI");
    openhd::rpi::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::log::get_default()->warn("LEDManager::set_rgb_led_status: Platform is Zero3W");
    openhd::zero3w::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::log::get_default()->warn("LEDManager::set_rgb_led_status: Platform is Radxa CM3");
    openhd::radxacm3::toggle_red_led(on);
  }
}

void openhd::LEDManager::set_secondary_led_status(int status) {
  openhd::log::get_default()->warn("LEDManager::set_secondary_led_status: status={}", status);
  const bool on = status != STATUS_ON;
  openhd::log::get_default()->warn("LEDManager::set_secondary_led_status: on={}", on);
  if (OHDPlatform::instance().is_rpi()) {
    openhd::log::get_default()->warn("LEDManager::set_secondary_led_status: Platform is RPI");
    openhd::rpi::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::log::get_default()->warn("LEDManager::set_secondary_led_status: Platform is Zero3W");
    openhd::zero3w::toggle_red_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::log::get_default()->warn("LEDManager::set_secondary_led_status: Platform is Radxa CM3");
    openhd::radxacm3::toggle_red_led(on);
  }
}

void openhd::LEDManager::set_primary_led_status(int status) {
  openhd::log::get_default()->warn("LEDManager::set_primary_led_status: status={}", status);
  const bool on = status != STATUS_ON;
  openhd::log::get_default()->warn("LEDManager::set_primary_led_status: on={}", on);
  if (OHDPlatform::instance().is_rpi()) {
    openhd::log::get_default()->warn("LEDManager::set_primary_led_status: Platform is RPI");
    openhd::rpi::toggle_green_led(on);
  } else if (OHDPlatform::instance().is_zero3w()) {
    openhd::log::get_default()->warn("LEDManager::set_primary_led_status: Platform is Zero3W");
    openhd::zero3w::toggle_green_led(on);
  } else if (OHDPlatform::instance().is_radxa_cm3()) {
    openhd::log::get_default()->warn("LEDManager::set_primary_led_status: Platform is Radxa CM3");
    openhd::radxacm3::toggle_green_led(on);
  }
}

openhd::LEDManager::LEDManager() {
  openhd::log::get_default()->warn("LEDManager::LEDManager()");
  // m_run= true;
  // m_manage_thread = std::make_unique<std::thread>(&LEDManager::loop, this);
}

openhd::LEDManager::~LEDManager() {
  openhd::log::get_default()->warn("LEDManager::~LEDManager()");
  /*m_run= false;
  if(m_manage_thread->joinable()){
    m_manage_thread->join();
  }
  m_manage_thread= nullptr;*/
}

void openhd::LEDManager::loop() {
  openhd::log::get_default()->warn("LEDManager::loop()");
  /*while (m_run){

  }*/
}

void openhd::LEDManager::set_status_okay() {
  openhd::log::get_default()->warn("LEDManager::set_status_loading()");
  std::string baseDir = "/sys/class/leds/";
  std::vector<std::string> folders = listLedFoldersWithBrightness(baseDir);

  openhd::log::get_default()->warn("LED folders with brightness file:");
  for (const std::string& folder : folders) {
    openhd::log::get_default()->warn("{}", folder);
  }
  turnOnAllLeds(folders, baseDir);
}

void openhd::LEDManager::set_status_loading() {
  openhd::log::get_default()->warn("LEDManager::set_status_loading()");
  std::string baseDir = "/sys/class/leds/";
  std::vector<std::string> folders = listLedFoldersWithBrightness(baseDir);

  openhd::log::get_default()->warn("LED folders with brightness file:");
  for (const std::string& folder : folders) {
    openhd::log::get_default()->warn("{}", folder);
  }

  // Turn off all detected LEDs
  turnOffAllLeds(folders, baseDir);
}

void openhd::LEDManager::set_status_error() {
  openhd::log::get_default()->warn("LEDManager::set_status_error()");
  set_primary_led_status(STATUS_ON);
  set_secondary_led_status(STATUS_ON);
  m_has_error = true;
  openhd::log::get_default()->warn("LEDManager::set_status_error(): m_has_error set to true");
}
