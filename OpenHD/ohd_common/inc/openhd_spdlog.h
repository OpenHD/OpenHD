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

// Helper to not spam the console with (error) messages, but rather have them sent out with a minimum amount
// of time in between messages
/*class LimitedRateLogger{
 public:
  LimitedRateLogger(std::shared_ptr<spdlog::logger> console1,std::chrono::milliseconds min_delay_between_messages):
  m_console(std::move(console1)),m_min_delay_between_messages(min_delay_between_messages){
  }
  template<typename... Args>
  void warn(fmt::format_string<Args...> fmt, Args &&... args){
    const auto elapsed_since_last=std::chrono::steady_clock::now()-m_last_log;
    if(elapsed_since_last<m_min_delay_between_messages){
      // drop message
      return;
    }
    m_last_log=std::chrono::steady_clock::now();
    m_console->log(spdlog::level::warn,fmt, std::forward<Args>(args)...);
  }
 private:
  std::shared_ptr<spdlog::logger> m_console;
  const std::chrono::milliseconds m_min_delay_between_messages;
  std::chrono::steady_clock::time_point m_last_log=std::chrono::steady_clock::now();
};*/

}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
