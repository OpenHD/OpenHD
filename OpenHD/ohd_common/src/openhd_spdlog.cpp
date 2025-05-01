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

#include "openhd_spdlog.h"

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <mutex>

#include "config_paths.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static openhd::log::MavlinkLogMessage safe_create(int level,
                                                  const std::string& message) {
  openhd::log::MavlinkLogMessage lmessage{};
  lmessage.level = static_cast<uint8_t>(level);
  strncpy((char*)lmessage.message, message.c_str(), 50);
  if (lmessage.message[49] != '\0') {
    lmessage.message[49] = '\0';
  }
  return lmessage;
}

// bridge between any logger and telemetry
// We send logs higher or equal to the warning log level out via udp
// such that they can be picked up by the telemetry module
namespace openhd::log::sink {

// Sinks the messages into a buffer
// For the telemetry thread to fetch
class MavlinkTelemetrySink : public spdlog::sinks::base_sink<std::mutex> {
 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    // log_msg is a struct containing the log entry info like level, timestamp,
    // thread id etc. msg.raw contains pre formatted log If needed (very likely
    // but not mandatory), the sink formats the message before sending it to its
    // final destination:
    if (msg.level >= spdlog::level::warn) {
      // We do not use the formatter here, since we are limited by 50 chars (and
      // the level, for example, is embedded already but not as a string).
      // spdlog::memory_buf_t formatted;
      // spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg,
      // formatted);
      const auto msg_string = fmt::to_string(msg.payload);
      const std::string msg_with_tag =
          fmt::format("{} {}", msg.logger_name, msg_string);
      const auto level = openhd::log::level_spdlog_to_mavlink(msg.level);
      auto tmp = safe_create(static_cast<int>(level), msg_with_tag);
      MavlinkLogMessageBuffer::instance().enqueue_log_message(tmp);
    }
  }
  void flush_() override {
    // std::cout << std::flush;
  }
};

}  // namespace openhd::log::sink

std::vector<openhd::log::MavlinkLogMessage>
openhd::log::MavlinkLogMessageBuffer::dequeue_log_messages() {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto ret = m_buffer;
  m_buffer.clear();
  return ret;
}
void openhd::log::MavlinkLogMessageBuffer::enqueue_log_message(
    openhd::log::MavlinkLogMessage message) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_buffer.size() > 10) {
    std::cerr << "Dropping log message:" << message.message << std::endl;
    return;
  }
  m_buffer.push_back(message);
}

openhd::log::MavlinkLogMessageBuffer&
openhd::log::MavlinkLogMessageBuffer::instance() {
  static MavlinkLogMessageBuffer singleton;
  return singleton;
}

std::shared_ptr<spdlog::logger> openhd::log::create_or_get(
    const std::string& logger_name) {
  static std::mutex logger_mutex2{};
  std::lock_guard<std::mutex> guard(logger_mutex2);
  auto ret = spdlog::get(logger_name);
  if (ret == nullptr) {
    auto created = spdlog::stdout_color_mt(logger_name);
    assert(created);
    if (OHDFilesystemUtil::exists("/usr/local/share/openhd/debug.txt")) {
      created->set_level(spdlog::level::debug);
    } else {
      created->set_level(spdlog::level::warn);
    }
    // Add the sink that sends out warning or higher via UDP
    // created->sinks().push_back(std::make_shared<openhd::log::sink::UdpTelemetrySink>());
    created->sinks().push_back(
        std::make_shared<openhd::log::sink::MavlinkTelemetrySink>());
    // This is for debugging for "where a fmt exception occurred"
    // spdlog::set_error_handler([](const std::string &msg) {
    //  std::cerr<<msg<<"\n;";
    //});
    return created;
  }
  return ret;
}

std::shared_ptr<spdlog::logger> openhd::log::get_default() {
  return create_or_get("default");
}

void openhd::log::log_via_mavlink(int level, std::string message) {
  auto tmp = safe_create(static_cast<int>(level), message);
  MavlinkLogMessageBuffer::instance().enqueue_log_message(tmp);
}

openhd::log::STATUS_LEVEL openhd::log::level_spdlog_to_mavlink(
    const spdlog::level::level_enum& level) {
  switch (level) {
    case spdlog::level::trace:
      return STATUS_LEVEL::DEBUG;
      break;
    case spdlog::level::debug:
      return STATUS_LEVEL::DEBUG;
      break;
    case spdlog::level::info:
      return STATUS_LEVEL::INFO;
      break;
    case spdlog::level::warn:
      return STATUS_LEVEL::WARNING;
    case spdlog::level::err:
      return STATUS_LEVEL::ERROR;
      break;
    case spdlog::level::critical:
      return STATUS_LEVEL::CRITICAL;
      break;
    default:
      break;
  }
  return STATUS_LEVEL::DEBUG;
}

void openhd::log::log_to_kernel(const std::string& message) {
  OHDUtil::run_command(fmt::format("echo \"{}\" > /dev/kmsg", message), {},
                       false);
}

void openhd::log::debug_log(const std::string& message) {
  openhd::log::get_default()->debug(message);
}
void openhd::log::info_log(const std::string& message) {
  openhd::log::get_default()->info(message);
}
void openhd::log::warning_log(const std::string& message) {
  openhd::log::get_default()->warn(message);
}
