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

#ifndef OPENHD_OPENHD_LED_H
#define OPENHD_OPENHD_LED_H

#include <atomic>
#include <thread>

namespace openhd {

/**
 * OpenHD uses 2 LEDs (green and red) for displaying status to the user.
 * Whether those LEDs exist or not depends on the hardware - here we abstract
 * that away.
 */
class LEDManager {
 public:
  static LEDManager& instance();
  static constexpr int STATUS_ON = 1;
  static constexpr int STATUS_OFF = 0;

  void set_primary_led_status(int status);
  void set_secondary_led_status(int status);
  void set_status_stopped(int status);

  // OpenHD is running and healthy
  void set_status_okay();
  // OpenHD is starting
  void set_status_loading();
  // OpenHD encountered an error
  void set_status_error();
  // OpenHD was closed
  void set_status_stopped();

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
