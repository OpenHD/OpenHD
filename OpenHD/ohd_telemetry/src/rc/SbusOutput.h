#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_SBUSOUTPUT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_SBUSOUTPUT_H_

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include "openhd_spdlog.h"

class SbusOutput {
 public:
  struct Options {
    bool enabled = false;
    std::string device;
    int update_rate_hz = 50;
  };

  explicit SbusOutput();
  ~SbusOutput();

  void configure(const Options& options);
  void update_channels(const std::array<uint16_t, 18>& channels);

 private:
  void start();
  void stop();
  void loop();
  bool open_port();
  void close_port();
  static std::array<uint8_t, 25> build_frame(
      const std::array<uint16_t, 18>& channels, bool stale);
  static uint16_t map_pwm_to_sbus(uint16_t pwm);
  std::array<uint16_t, 18> get_channels_copy();

 private:
  std::shared_ptr<spdlog::logger> m_console;
  Options m_options{};
  std::atomic<bool> m_running{false};
  std::thread m_thread;
  int m_fd = -1;

  std::mutex m_mutex;
  std::array<uint16_t, 18> m_channels{};
  std::atomic<int64_t> m_last_update_ms{0};
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_SBUSOUTPUT_H_
