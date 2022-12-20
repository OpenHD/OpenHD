//
// Created by consti10 on 27.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_

// The goal is to eventually use spdlog throughout all openhd modules, but have
// specific logger(s) on a module basis such that we can enable / disable logging
// for a specific module (e.g. ohd_video: set log level to debug / info) when debugging ohd_video.

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include <mutex>
#include <utility>

#include "openhd-spdlog-tele-sink.h"

namespace spd = spdlog;

namespace openhd::log{

// Note: the _mt loggers have threadsafety by design already, but we need to make sure to crete the instance only once
// For some reason there is no helper for that in speeddlog / i haven't found it yet

// Thread-safe but recommended to store result in an intermediate variable
static std::shared_ptr<spdlog::logger> create_or_get(const std::string& logger_name){
  static std::mutex logger_mutex2{};
  std::lock_guard<std::mutex> guard(logger_mutex2);
  auto ret = spdlog::get(logger_name);
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt(logger_name);
    assert(created);
    created->set_level(spd::level::debug);
    // Add the sink that sends out warning or higher via UDP
    created->sinks().push_back(std::make_shared<openhd::log::sink::UdpTelemetrySink>());
    //spdlog::set_error_handler([](const std::string& msg) {
    //  std::cerr << "my err handler: " << msg << std::endl;
    //});
    return created;
  }
  return ret;
}

static std::shared_ptr<spdlog::logger> get_default() {
  return create_or_get("default");
}

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
