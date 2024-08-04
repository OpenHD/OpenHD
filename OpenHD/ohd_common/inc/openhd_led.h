#ifndef OPENHD_OPENHD_LED_H
#define OPENHD_OPENHD_LED_H

#include <atomic>
#include <thread>

namespace openhd {

/**
 * OpenHD uses 2 LEDs (green and red) for displaying status to the user.
 * Whether those LEDs exist or not depends on the hardware - here we abstract that
 * away.
 */
class LEDManager {
 public:
  static LEDManager& instance();
  static constexpr int STATUS_OFF = 0;
  static constexpr int STATUS_ON = 1;
  
  void set_red_led_status(int status);
  void set_green_led_status(int status);

  // OpenHD is running and healthy
  void set_status_okay();
  // OpenHD is starting
  void set_status_loading();
  // OpenHD encountered an error
  void set_status_error();

 private:
  LEDManager();
  ~LEDManager();
  
  void start_loading_thread();
  void stop_loading_thread();
  void loading_loop();
  void blink_okay();
  void blink_loading();
  void blink_error();

  std::unique_ptr<std::thread> m_loading_thread;
  std::atomic<bool> m_running;
  std::atomic<bool> m_has_error;
  std::atomic<bool> m_is_loading;
};

}  // namespace openhd

#endif  // OPENHD_OPENHD_LED_H
