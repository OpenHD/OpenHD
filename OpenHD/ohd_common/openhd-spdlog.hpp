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

// Note: the _mt loggers are threadsafe by design already, but we need to make sure to cretae the instance only once
// For some reason there is no helper for that in spdlog / i haven't found it yet

// Thread-safe but recommended to store result in an intermediate variable
static std::shared_ptr<spdlog::logger> get_video() {
  static std::mutex logger_mutex{};
  std::lock_guard<std::mutex> guard(logger_mutex);
  auto ret = spdlog::get("ohd_video");
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt("ohd_video");
    assert(created);
    return created;
  }
  return ret;
}

static std::shared_ptr<spdlog::logger> get_default() {
  static std::mutex logger_mutex{};
  std::lock_guard<std::mutex> guard(logger_mutex);
  auto ret = spdlog::get("ohd_default");
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt("ohd_default");
    assert(created);
    created->set_level(spd::level::debug);
    return created;
  }
  return ret;
}
}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_SPDLOG_HPP_
