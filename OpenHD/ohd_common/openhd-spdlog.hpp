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
#include <mutex>

namespace spd = spdlog;

namespace openhd::loggers {

// Note: the _mt loggers are threadsafe by design already, but we need to make sure to crete the instance only once
// For some reason there is no helper for that in spdlog / i haven't found it yet

// Thread-safe but recommended to store result in an intermediate variable

static std::shared_ptr<spdlog::logger> create_or_get(const std::string& logger_name){
  static std::mutex logger_mutex2{};
  std::lock_guard<std::mutex> guard(logger_mutex2);
  auto ret = spdlog::get(logger_name);
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt(logger_name);
    assert(created);
    return created;
  }
  return ret;
}

static std::shared_ptr<spdlog::logger> get_default() {
  return create_or_get("ohd_default");
}

}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
