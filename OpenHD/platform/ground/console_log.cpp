#include "openhd_spdlog.h"
#include "ground_config.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <atomic>
#include <fstream>

namespace {
std::mutex log_mutex;
std::filesystem::path log_directory;
std::atomic<bool> persistent{false};
}
std::shared_ptr<spdlog::logger> openhd::log::create_or_get(const std::string& name) {
  static std::mutex mutex;
  std::lock_guard<std::mutex> guard(mutex);
  auto logger = spdlog::get(name);
  if (!logger) logger = spdlog::stdout_color_mt(name);
  logger->set_level(spdlog::level::debug);
  return logger;
}
bool openhd::log::persistent_logging_enabled() { return persistent.load(); }
bool openhd::log::set_persistent_logging_enabled(bool enabled, bool persist_preference) {
  std::lock_guard<std::mutex> guard(log_mutex);
  if (log_directory.empty()) return false;
  std::error_code error;
  std::filesystem::create_directories(log_directory, error);
  if (error) return false;
  if (persist_preference) {
    if (enabled) { std::ofstream marker(log_directory / "enabled"); marker << "1\n"; marker.close(); if (!marker) return false; }
    else { std::filesystem::remove(log_directory / "enabled", error); if (error) return false; }
  }
  persistent.store(enabled); return true;
}
void openhd::ground::configure_log_directory(const std::filesystem::path& path) {
  std::lock_guard<std::mutex> guard(log_mutex);
  log_directory = std::filesystem::absolute(path);
  persistent.store(std::filesystem::exists(log_directory / "enabled"));
#ifdef _WIN32
  _putenv_s("OPENHD_LOG_EXPORT_DIR", log_directory.string().c_str());
#else
  setenv("OPENHD_LOG_EXPORT_DIR", log_directory.string().c_str(), 1);
#endif
}
void openhd::ground::record_console_line(const std::string& line) {
  if (!persistent.load()) return;
  std::lock_guard<std::mutex> guard(log_mutex);
  std::error_code error;
  std::filesystem::create_directories(log_directory, error);
  if (error) return;
  const auto file = log_directory / "ground.log";
  if (std::filesystem::file_size(file, error) > 10 * 1024 * 1024 && !error) {
    const auto old = log_directory / "ground.previous.log";
    std::filesystem::remove(old, error); error.clear(); std::filesystem::rename(file, old, error);
  }
  std::ofstream output(file, std::ios::app); output << line << '\n';
}
