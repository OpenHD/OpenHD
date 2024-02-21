//
// Created by consti10 on 08.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

namespace mavsdk {
class YLogger {
 public:
  // explicit OpenHDLogger(const STATUS_LEVEL level=STATUS_LEVEL_DEBUG,const
  // std::string& tag=""):
  //   _status_level(level),_tag(tag) {}
  YLogger() { m_console = openhd::log::create_or_get("mavsdk"); };
  ~YLogger() {
    const auto tmp = stream.str();
    log_message(tmp);
  }
  YLogger(const YLogger& other) = delete;
  // the non-member function operator<< will now have access to private members
  template <typename T>
  friend YLogger& operator<<(YLogger& record, T&& t);

 private:
  std::stringstream stream;
  std::shared_ptr<spdlog::logger> m_console;
  // Checks for a newline, and if detected logs the message immediately and then
  // clears it.
  void log_immediately_on_newline() {
    const auto tmp = stream.str();
    if (!tmp.empty() && tmp.back() == '\n') {
      log_message(tmp);
      stream.str("");
    }
  }
  void log_message(const std::string& message) {
    if (message.empty()) return;
    m_console->debug(message);
  }
};

template <typename T>
YLogger& operator<<(YLogger& record, T&& t) {
  record.stream << std::forward<T>(t);
  // If we detect a newline, log immediately, not delayed when going out of
  // scope to mimic std::cout behaviour
  record.log_immediately_on_newline();
  return record;
}
template <typename T>
YLogger& operator<<(YLogger&& record, T&& t) {
  // This just uses the operator declared above.
  record << std::forward<T>(t);
  return record;
}
}  // namespace mavsdk

static mavsdk::YLogger LogDebug() { return {}; }

static mavsdk::YLogger LogWarn() { return {}; }

static mavsdk::YLogger LogErr() { return {}; }

//}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_
