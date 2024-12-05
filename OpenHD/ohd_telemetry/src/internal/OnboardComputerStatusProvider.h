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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_

#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "../mav_include.h"
#include "ina219.h"
#include "openhd_platform.h"

/**
 * This class nicely hides away all the (nasty) reading of the onboard computer
 * status (clock speed, temperature,..) A status can be queried any time,
 * basically atomically.
 *
 * More info:
 * We need one thread for the CPU usage (workaround) and
 * for some reason, running all those vcgencmd's on rpi for figuring out the
 * current clock speed can also block for a significant amount of time
 * (significant enough that it is a bad idea to call them from the "main"
 * telemetry thread. This class decouples these data generation steps from the
 * main telemetry thread. We do not care about latency at all on these
 * statistics, so we can easily do those stats using a producer / consumer
 * pattern
 */
class OnboardComputerStatusProvider {
 public:
  /**
   * @param platform platform we are running on
   * @param enable : calculating cpu usage and more for whatever reason can use
   * a lot of CPU resources on its own
   * - disable for testing
   */
  explicit OnboardComputerStatusProvider(bool enable = true);
  ~OnboardComputerStatusProvider();
  // Thread-safe, should never block for a significant amount of time
  mavlink_onboard_computer_status_t get_current_status();
  // utility for OHDMainComponent,also thread-safe
  // Extradata: Kinda dirty, more info about the uart OpenHD air unit <-> FC
  struct ExtraUartInfo {
    int16_t fc_sys_id;
    uint8_t operating_mode;
  };
  MavlinkMessage get_current_status_as_mavlink_message(
      uint8_t sys_id, uint8_t comp_id,
      const std::optional<ExtraUartInfo>& extra_uart);

 private:
  const bool m_enable;
  std::mutex m_curr_onboard_computer_status_mutex;
  mavlink_onboard_computer_status_t m_curr_onboard_computer_status{};
  // Power monitoring via ina219. Optional, not hot swappable, if there is no
  // ina219, a warning is logged once and then no values are read anymore
  INA219 m_ina_219;
  bool m_ina219_warning_logged = false;
  // One thread for calculating the CPU usage
  std::unique_ptr<std::thread> m_calculate_cpu_usage_thread;
  std::unique_ptr<std::thread> m_calculate_other_thread;
  bool terminate = false;
  // Note: we need to run this in its own thread, such that "top" can do its
  // magic over a longer duration
  void calculate_cpu_usage_until_terminate();
  // Extra thread for "the rest"
  void calculate_other_until_terminate();
  void ina219_log_warning_once(int curr_ina219_voltage);
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_
