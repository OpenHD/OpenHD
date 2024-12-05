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
