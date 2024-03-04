//
// Created by consti10 on 01.02.24.
//

#ifndef OPENHD_OPENHD_LED_H
#define OPENHD_OPENHD_LED_H

#include <atomic>
#include <thread>
namespace openhd {

/**
 * OpenHD uses 2 leds (green and red) for displaying 'stuff' to the user.
 * Weather those leds exists or not depends on the HW - here we abstract that
 * away.
 */
class LEDManager {
 public:
  static LEDManager& instance();
  static constexpr int STATUS_OFF = 0;
  static constexpr auto STATUS_ON = 1;
  void set_red_led_status(int status);
  void set_green_led_status(int status);

 public:
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

 private:
  // bool m_run= true;
  // std::unique_ptr<std::thread> m_manage_thread;
  bool m_has_error = false;
};

}  // namespace openhd

class openhd_led {};

#endif  // OPENHD_OPENHD_LED_H
