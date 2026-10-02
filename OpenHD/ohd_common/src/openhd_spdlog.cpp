#include <unistd.h>
#include "openhd_ncurses_ui.h"
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
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include "config_paths.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static constexpr size_t MAX_BUFFERED_MAVLINK_LOG_MESSAGES = 80;

namespace {
constexpr std::size_t kPersistentLogSize = 10 * 1024 * 1024;
constexpr std::size_t kPersistentLogFiles = 3;

class RuntimeFileSink : public spdlog::sinks::base_sink<std::mutex> {
 public:
  void set_target(std::shared_ptr<spdlog::sinks::sink> target) {
    std::lock_guard<std::mutex> guard(mutex_);
    if (target_) target_->flush();
    target_ = std::move(target);
  }

 protected:
  void sink_it_(const spdlog::details::log_msg& message) override {
    if (target_) target_->log(message);
  }
  void flush_() override {
    if (target_) target_->flush();
  }

 private:
  std::shared_ptr<spdlog::sinks::sink> target_;
};

struct PersistentLogState {
  bool initialized = false;
  std::atomic<bool> enabled{false};
  std::string directory;
  std::shared_ptr<RuntimeFileSink> openhd;
  std::shared_ptr<RuntimeFileSink> camera;
  std::shared_ptr<RuntimeFileSink> other;
  std::unordered_map<std::string, spdlog::level::level_enum> previous_levels;
  uint64_t next_listener_id = 1;
  std::unordered_map<uint64_t, openhd::log::PersistentLoggingListener> listeners;
};

PersistentLogState& persistent_state() {
  static auto* state = new PersistentLogState();
  return *state;
}

std::mutex& logger_mutex() {
  static auto* mutex = new std::mutex();
  return *mutex;
}

std::string lowercase(std::string value) {
  for (auto& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return value;
}

std::shared_ptr<spdlog::sinks::sink> sink_for_logger(const std::string& name) {
  auto& state = persistent_state();
  const auto lower = lowercase(name);
  if (lower.find("cam") != std::string::npos || lower.find("video") != std::string::npos ||
      lower.find("gst") != std::string::npos || lower.find("mpp") != std::string::npos ||
      lower == "v_air" || lower == "v_gnd" || lower.find("rtp") != std::string::npos)
    return state.camera;
  if (lower.find("wb") != std::string::npos || lower.find("wifi") != std::string::npos ||
      lower.find("tele") != std::string::npos || lower.find("mav") != std::string::npos ||
      lower.find("serial") != std::string::npos || lower.find("udp") != std::string::npos ||
      lower.find("tcp") != std::string::npos || lower.find("sbus") != std::string::npos ||
      lower.find("adsb") != std::string::npos || lower.find("audio") != std::string::npos ||
      lower.find("artosyn") != std::string::npos || lower.find("eth_") != std::string::npos)
    return state.other;
  return state.openhd;
}

void attach_persistent_sink(const std::shared_ptr<spdlog::logger>& logger) {
  auto& state = persistent_state();
  if (!state.initialized) return;
  if (auto sink = sink_for_logger(logger->name())) {
    const auto already_attached =
        std::find(logger->sinks().begin(), logger->sinks().end(), sink) !=
        logger->sinks().end();
    if (!already_attached) logger->sinks().push_back(sink);
  }
  if (state.enabled.load()) {
    state.previous_levels.emplace(logger->name(), logger->level());
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::info);
  }
}

std::filesystem::path log_control_directory() {
  if (const char* configured = std::getenv("OPENHD_LOG_CONTROL_DIR")) {
    if (*configured) return configured;
  }
  return "/Config/openhd";
}

std::filesystem::path development_image_marker() {
  if (const char* configured = std::getenv("OPENHD_DEV_IMAGE_MARKER")) {
    if (*configured) return configured;
  }
  return "/usr/local/share/openhd/dev_image.txt";
}

bool persist_preference(bool enabled) {
  std::error_code error;
  const auto directory = log_control_directory();
  std::filesystem::create_directories(directory, error);
  if (error) return false;
  std::filesystem::remove(
      directory / (enabled ? "disable_logs.txt" : "enable_logs.txt"), error);
  error.clear();
  std::ofstream marker(directory /
                       (enabled ? "enable_logs.txt" : "disable_logs.txt"));
  marker << (enabled ? "enabled" : "disabled") << " by runtime control\n";
  return static_cast<bool>(marker);
}

bool configure_persistent_logging(bool enabled) {
  auto& state = persistent_state();
  if (state.enabled.load() == enabled) return true;
  if (!enabled) {
    state.enabled.store(false);
    spdlog::apply_all([&](const std::shared_ptr<spdlog::logger>& logger) {
      const auto previous = state.previous_levels.find(logger->name());
      if (previous != state.previous_levels.end()) logger->set_level(previous->second);
    });
    state.previous_levels.clear();
    state.openhd->set_target(nullptr);
    state.camera->set_target(nullptr);
    state.other->set_target(nullptr);
    return true;
  }
  try {
    state.directory = openhd::log::persistent_log_directory();
    OHDFilesystemUtil::create_directories(state.directory);
    const auto make_sink = [&](const char* filename) {
      auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          state.directory + "/" + filename, kPersistentLogSize,
          kPersistentLogFiles, false);
      sink->set_pattern("%Y-%m-%d %H:%M:%S.%e [%n] [%l] %v");
      return sink;
    };
    state.openhd->set_target(make_sink("openhd.log"));
    state.camera->set_target(make_sink("camera.log"));
    state.other->set_target(make_sink("other.log"));
    state.enabled.store(true);
    spdlog::apply_all([](const std::shared_ptr<spdlog::logger>& logger) {
      attach_persistent_sink(logger);
    });
    return true;
  } catch (const std::exception& error) {
    state.openhd->set_target(nullptr);
    state.camera->set_target(nullptr);
    state.other->set_target(nullptr);
    state.enabled.store(false);
    std::cerr << "Cannot enable persistent OpenHD logs: " << error.what() << '\n';
    return false;
  }
}
}  // namespace

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

class NcursesSink : public spdlog::sinks::base_sink<std::mutex> {
 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    openhd::ui::ncurses_log(std::string(msg.logger_name.data(), msg.logger_name.size()), static_cast<int>(msg.level), fmt::to_string(msg.payload));
    if (!openhd::ui::ncurses_active()) {
      fallback_.log(msg);
    }
  }
  void flush_() override { fallback_.flush(); }
 private:
  spdlog::sinks::stdout_color_sink_mt fallback_;
};


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
  if (m_buffer.size() >= MAX_BUFFERED_MAVLINK_LOG_MESSAGES) {
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
  std::lock_guard<std::mutex> guard(logger_mutex());
  auto ret = spdlog::get(logger_name);
  if (ret == nullptr) {
    auto created = std::make_shared<spdlog::logger>(logger_name);
    spdlog::register_logger(created);
    created->sinks().push_back(std::make_shared<openhd::log::sink::NcursesSink>());
    assert(created);
    if (openhd::ui::ncurses_active() ||
        OHDFilesystemUtil::exists("/usr/local/share/openhd/debug.txt")) {
      created->set_level(spdlog::level::debug);
    } else {
      created->set_level(spdlog::level::warn);
    }
    // Add the sink that sends out warning or higher via UDP
    // created->sinks().push_back(std::make_shared<openhd::log::sink::UdpTelemetrySink>());
    created->sinks().push_back(
        std::make_shared<openhd::log::sink::MavlinkTelemetrySink>());
    attach_persistent_sink(created);
    // This is for debugging for "where a fmt exception occurred"
    // spdlog::set_error_handler([](const std::string &msg) {
    //  std::cerr<<msg<<"\n;";
    //});
    return created;
  }
  return ret;
}

std::string openhd::log::persistent_log_directory() {
  if (const char* configured = std::getenv("OPENHD_PERSISTENT_LOG_DIR")) {
    if (*configured) return configured;
  }
  return (std::filesystem::path(getVideoPath()) / "logs" / "openhd").string();
}

bool openhd::log::persistent_logging_enabled() {
  return persistent_state().enabled.load();
}

void openhd::log::attach_persistent_logging_sink(
    const std::shared_ptr<spdlog::logger>& logger) {
  std::lock_guard<std::mutex> guard(logger_mutex());
  attach_persistent_sink(logger);
}

void openhd::log::initialize_persistent_logging() {
  std::lock_guard<std::mutex> guard(logger_mutex());
  auto& state = persistent_state();
  if (state.initialized) return;
  state.initialized = true;
  state.openhd = std::make_shared<RuntimeFileSink>();
  state.camera = std::make_shared<RuntimeFileSink>();
  state.other = std::make_shared<RuntimeFileSink>();
  spdlog::apply_all([](const std::shared_ptr<spdlog::logger>& logger) {
    attach_persistent_sink(logger);
  });
  const auto control = log_control_directory();
  const bool disabled = OHDFilesystemUtil::exists(
      (control / "disable_logs.txt").string());
  const bool development = OPENHD_DEVELOPMENT_BUILD ||
      OHDFilesystemUtil::exists(development_image_marker().string());
  const bool requested =
      OHDFilesystemUtil::exists((control / "enable_logs.txt").string()) ||
      std::getenv("OPENHD_FORCE_PERSISTENT_LOGS") != nullptr ||
      std::getenv("OPENHD_PERSISTENT_LOG_DIR") != nullptr;
  configure_persistent_logging(!disabled && (development || requested));
}

bool openhd::log::set_persistent_logging_enabled(bool enabled,
                                                  bool persist) {
  std::vector<PersistentLoggingListener> listeners;
  {
    std::lock_guard<std::mutex> guard(logger_mutex());
    auto& state = persistent_state();
    if (!state.initialized) return false;
    if (!configure_persistent_logging(enabled)) return false;
    if (persist && !persist_preference(enabled)) {
      std::cerr << "Cannot persist OpenHD logging preference\n";
    }
    for (const auto& [id, listener] : state.listeners) listeners.push_back(listener);
  }
  for (const auto& listener : listeners) listener(enabled);
  return true;
}

uint64_t openhd::log::add_persistent_logging_listener(
    PersistentLoggingListener listener) {
  std::lock_guard<std::mutex> guard(logger_mutex());
  auto& state = persistent_state();
  const auto id = state.next_listener_id++;
  state.listeners.emplace(id, std::move(listener));
  return id;
}

void openhd::log::remove_persistent_logging_listener(uint64_t listener_id) {
  std::lock_guard<std::mutex> guard(logger_mutex());
  persistent_state().listeners.erase(listener_id);
}

void openhd::log::enable_debug_logging_for_dashboard() {
  spdlog::apply_all([](const std::shared_ptr<spdlog::logger>& logger) {
    logger->set_level(spdlog::level::debug);
  });
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
