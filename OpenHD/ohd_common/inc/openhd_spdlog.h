//
// Created by consti10 on 27.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_

// The goal is to eventually use spdlog throughout all openhd modules, but have
// specific logger(s) on a module basis such that we can enable / disable logging
// for a specific module (e.g. ohd_video: set log level to debug / info) when debugging ohd_video.

//#include <spdlog/fwd.h>
//#include <spdlog/fmt/fmt.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
//# define FMT_STRING(s) s

namespace openhd::log{

// Note: the _mt loggers have threadsafety by design already, but we need to make sure to crete the instance only once
// For some reason there is no helper for that in speeddlog / i haven't found it yet

// Thread-safe but recommended to store result in an intermediate variable
std::shared_ptr<spdlog::logger> create_or_get(const std::string& logger_name);

// Uses the thread-safe create_or_get -> slower than using the intermediate variable approach,
// but sometimes you just don't care about that.
std::shared_ptr<spdlog::logger> get_default();

// By default, only messages of level warn or higher are forwarded via mavlink (and then shown in QOpenHD).
// Use this if you want to show a non-warning message in QOpenHD.
void log_via_mavlink(int level,std::string message);

struct MavlinkLogMessage {
  uint8_t level;
  // MUST be null-terminated
  uint8_t message[50];
};

class MavlinkLogMessageBuffer{
 public:
  // Thread-safe
  // Enqueues a log message for the telemetry thread to fetch
  std::vector<MavlinkLogMessage> dequeue_log_messages();
  // Thread-safe
  // Dequeues buffered telemetry log messages,
  // called in regular intervals by the telemetry thread
  void enqueue_log_message(MavlinkLogMessage message);
  // We only have one instance of this class inside openhd
  static MavlinkLogMessageBuffer& instance();
 private:
  std::mutex m_mutex;
  std::vector<MavlinkLogMessage> m_buffer;
};

// these match the mavlink SEVERITY_LEVEL enum, but this code should not depend on
// the mavlink headers
// See https://mavlink.io/en/messages/common.html#MAV_SEVERITY
enum class STATUS_LEVEL {
  EMERGENCY = 0,
  ALERT,
  CRITICAL,
  ERROR,
  WARNING,
  INFO,
  NOTICE,
  DEBUG
};

STATUS_LEVEL level_spdlog_to_mavlink(const spdlog::level::level_enum& level);
}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
