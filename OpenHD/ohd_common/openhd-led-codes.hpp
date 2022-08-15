//
// Created by consti10 on 15.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LED_CODES_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LED_CODES_H_

#include "openhd-led-pi.hpp"

namespace openhd {

// R.N only to show no connected wifi card, which requires a reboot to fix.
class LEDBlinker {
 public:
  // toggle red led off, wait for delay, then toggle it on,wait for delay
  static void rpi_red_led_on_off_delayed(const std::chrono::milliseconds &delay) {
	rpi::toggle_red_led(false);
	std::this_thread::sleep_for(delay);
	rpi::toggle_red_led(true);
	std::this_thread::sleep_for(delay);
  }
  // One on / off sequence is often not enough signal for the user, repeat the sequence for a given amount of time
  // blink red led in X second intervals, runs for duration seconds. Defaults to infinity (note the calling thread will be blocked then)
  void blink_red_led(const std::string &message, const std::chrono::seconds duration = DURATION_INFINITY) const {
	const auto start = std::chrono::steady_clock::now();
	while ((std::chrono::steady_clock::now() - start) <= duration) {
	  LOGE << message;
	  if (_platform.platform_type == PlatformType::RaspberryPi) {
		rpi_red_led_on_off_delayed(std::chrono::seconds(1));
	  } else {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	  }
	}
  }
  // For running in its own thread
  // Make sure to store a reference to this class, otherwise destruct will fail since thread is still running.
  explicit LEDBlinker(OHDPlatform platform,
					  std::string message,
					  const std::chrono::seconds duration = DURATION_INFINITY) :
	  _message(std::move(message)), _duration(duration), _platform(platform) {
	_blink_thread = std::make_unique<std::thread>(&LEDBlinker::run, this);
  }
  static constexpr auto DURATION_INFINITY = std::chrono::seconds(100 * 100 * 100 * 100);
 private:
  void run() {
	blink_red_led(_message, _duration);
  }
  const std::string _message;
  const std::chrono::seconds _duration;
  std::unique_ptr<std::thread> _blink_thread = nullptr;
  const OHDPlatform _platform;
};
}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_LED_CODES_H_
