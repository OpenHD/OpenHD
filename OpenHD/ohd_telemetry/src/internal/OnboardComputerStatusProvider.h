//
// Created by consti10 on 30.10.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_

#include "../mav_include.h"
#include "OnboardComputerStatus.hpp"
#include "../ohd_common/openhd-platform.hpp"

#include <thread>
#include <mutex>
#include <memory>

// We need one thread for the CPU usage (workaround) and
// for some reason, running all those vcgencmd's on rpi for figuring out the current clock
// speed can also block for a significant amount of time (significant enough that it is a bad idea
// to call them from the "main" telemetry thread.
// This class decouples these data generation steps from the main telemetry thread. We do not care
// about latency at all on these statistics, so we can easily do those stats using a
// producer / consumer pattern
class OnboardComputerStatusProvider {
 public:
  explicit OnboardComputerStatusProvider(OHDPlatform platform);
  ~OnboardComputerStatusProvider();
  // Thread-safe, should never block for a significant amount of time
  mavlink_onboard_computer_status_t get_current_status();
  // utility for OHDMainComponent,also thread-safe
  std::vector<MavlinkMessage> get_current_status_as_mavlink_message(uint8_t sys_id,uint8_t comp_id);
 private:
  const OHDPlatform m_platform;
  std::mutex m_curr_onboard_computer_status_mutex;
  mavlink_onboard_computer_status_t m_curr_onboard_computer_status{};
  // One thread for calculating the CPU usage
  std::unique_ptr<std::thread> m_calculate_cpu_usage_thread;
  std::unique_ptr<std::thread> m_calculate_other_thread;
  bool terminate= false;
  // Note: we need to run this in its own thread, such that "top" can do its magic over a longer duration
  void calculate_cpu_usage_until_terminate();
  // Extra thread for "the rest"
  void calculate_other_until_terminate();
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_ONBOARDCOMPUTERSTATUSPROVIDER_H_
