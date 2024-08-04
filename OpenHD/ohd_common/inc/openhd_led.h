// Created by consti10 on 01.02.24.
//

#ifndef OPENHD_OPENHD_LED_H
#define OPENHD_OPENHD_LED_H

#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

namespace openhd {

/**
 * OpenHD uses 2 LEDs (green and red) for displaying 'stuff' to the user.
 * Whether those LEDs exist or not depends on the HW - here we abstract that
 * away.
 */
class LEDManager {
 public:
  static LEDManager& instance();
  static constexpr int STATUS_OFF = 0;
  static constexpr int STATUS_ON = 1;

  void set_secondary_led_status(int status);
  void set_primary_led_status(int status);
  void set_aux_led_status(int status);
  void set_rgb_led_status(int status, int color);
  
  // OpenHD is running and healthy
  void set_status_okay();
  // OpenHD is starting
  void set_status_loading();
  // OpenHD encountered an error
  void set_status_error();

 private:
  explicit LEDManager();
  ~LEDManager();
  void loop();
  void set_led_status(bool on, const std::function<void(bool)>& toggle_led_fn);

  bool m_has_error = false;
};

}  // namespace openhd

std::vector<std::string> listLedFoldersWithBrightness(const std::string& baseDir);
void setLedBrightness(const std::vector<std::string>& ledFolders, const std::string& baseDir, const std::string& value);
void turnOffAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir);
void turnOnAllLeds(const std::vector<std::string>& ledFolders, const std::string& baseDir);

namespace openhd::rpi {
    void toggle_secondary_led(bool on);
    void toggle_primary_led(bool on);
    void toggle_led_delayed(bool on, const std::chrono::milliseconds& delay);
}

namespace openhd::zero3w {
    void toggle_secondary_led(bool on);
    void toggle_primary_led(bool on);
}

namespace openhd::radxacm3 {
    void toggle_secondary_led(bool on);
    void toggle_primary_led(bool on);
}

#endif  // OPENHD_OPENHD_LED_H
