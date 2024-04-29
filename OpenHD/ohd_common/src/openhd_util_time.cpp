//
// Created by consti10 on 21.02.24.
//

#include "openhd_util_time.h"

#include <sstream>

std::string openhd::util::verbose_timespan(
    const std::chrono::steady_clock::duration& duration) {
  if (duration > std::chrono::seconds(1)) {
    const double elapsed_ms =
        (double)std::chrono::duration_cast<std::chrono::milliseconds>(duration)
            .count();
    const double elapsed_seconds = elapsed_ms / 1000.0;
    std::stringstream ss;
    ss.precision(2);
    ss << elapsed_seconds << "s";
    return ss.str();
  }
  const double elapsed_us =
      (double)std::chrono::duration_cast<std::chrono::microseconds>(duration)
          .count();
  const double elapsed_ms = elapsed_us / 1000.0;
  std::stringstream ss;
  ss.precision(2);
  ss << elapsed_ms << "ms";
  return ss.str();
}

int openhd::util::steady_clock_time_epoch_ms() {
  const auto now = std::chrono::steady_clock::now();
  const std::chrono::milliseconds now_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch());
  return now_ms.count();
}

std::string openhd::util::time_readable(
    const std::chrono::steady_clock::duration& dur) {
  const auto durAbsolute = std::chrono::abs(dur);
  if (durAbsolute >= std::chrono::seconds(1)) {
    // More than one second, print as decimal with ms resolution.
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    return std::to_string(static_cast<float>(ms) / 1000.0f) + "s";
  }
  if (durAbsolute >= std::chrono::milliseconds(1)) {
    // More than one millisecond, print as decimal with us resolution
    const auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
    return std::to_string(static_cast<float>(us) / 1000.0f) + "ms";
  }
  if (durAbsolute >= std::chrono::microseconds(1)) {
    // More than one microsecond, print as decimal with ns resolution
    const auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    return std::to_string(static_cast<float>(ns) / 1000.0f) + "us";
  }
  const auto ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  return std::to_string(ns) + "ns";
}

std::string openhd::util::time_readable_ns(uint64_t nanoseconds) {
  return time_readable(std::chrono::nanoseconds(nanoseconds));
}

uint32_t openhd::util::get_micros(std::chrono::nanoseconds ns) {
  return static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(ns).count());
}

// Until we have std::atomic<uint64_t>
class ThreadSafeINT64_t {
 public:
  int64_t load() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return value;
  }
  void store(int64_t v) {
    std::lock_guard<std::mutex> lock(m_mutex);
    value = v;
  }

 private:
  std::mutex m_mutex;
  int64_t value = 0;
};
static ThreadSafeINT64_t& get_air_ts() {
  static ThreadSafeINT64_t holder;
  return holder;
}

void openhd::util::store_air_unit_time_offset_us(int64_t offset_us) {
  get_air_ts().store(offset_us);
}
int64_t openhd::util::get_air_unit_time_offset_us() {
  return get_air_ts().load();
}
