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

#include "openhd_util_time.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <sstream>

#include "openhd_spdlog.h"

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

namespace {

constexpr uint64_t kUnixUsPerSecond = 1000ULL * 1000ULL;
constexpr uint64_t kMinTrustedUnixUs = 1704067200ULL * kUnixUsPerSecond;
constexpr uint64_t kMaxTrustedUnixUs = 2114380800ULL * kUnixUsPerSecond;
constexpr int64_t kMinStepOffsetUs = 6LL * 60LL * 60LL * 1000LL * 1000LL;

struct GpsTimeSyncState {
  std::mutex mutex;
  bool adjusted = false;
  bool logged_small_offset = false;
  bool logged_invalid = false;
  bool logged_set_failure = false;
};

GpsTimeSyncState& gps_time_sync_state() {
  static GpsTimeSyncState state;
  return state;
}

bool is_sane_unix_time_us(uint64_t unix_time_us) {
  return unix_time_us >= kMinTrustedUnixUs && unix_time_us <= kMaxTrustedUnixUs;
}

uint64_t system_time_unix_us() {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

}  // namespace

bool openhd::util::maybe_adjust_system_time_from_unix_us(
    uint64_t unix_time_us, const std::string& source) {
  auto& state = gps_time_sync_state();
  std::lock_guard<std::mutex> lock(state.mutex);
  if (state.adjusted) {
    return true;
  }
  auto logger = openhd::log::create_or_get("time_sync");
  if (!is_sane_unix_time_us(unix_time_us)) {
    if (!state.logged_invalid) {
      logger->warn("Ignoring invalid {} time {}us", source, unix_time_us);
      state.logged_invalid = true;
    }
    return false;
  }
  const uint64_t current_us = system_time_unix_us();
  const int64_t delta_us = static_cast<int64_t>(unix_time_us) -
                           static_cast<int64_t>(current_us);
  if (std::llabs(delta_us) < kMinStepOffsetUs) {
    if (!state.logged_small_offset) {
      logger->info(
          "System time close enough to {} time (offset {}s), not stepping",
          source, delta_us / static_cast<int64_t>(kUnixUsPerSecond));
      state.logged_small_offset = true;
    }
    return true;
  }

#ifdef __linux__
  timespec ts{};
  ts.tv_sec = static_cast<time_t>(unix_time_us / kUnixUsPerSecond);
  ts.tv_nsec =
      static_cast<long>((unix_time_us % kUnixUsPerSecond) * 1000ULL);
  if (clock_settime(CLOCK_REALTIME, &ts) != 0) {
    if (!state.logged_set_failure) {
      logger->warn("Failed to set system time from {}: {}", source,
                   std::strerror(errno));
      state.logged_set_failure = true;
    }
    return true;
  }
  state.adjusted = true;
  logger->warn("Stepped system time from {} by {}s", source,
               delta_us / static_cast<int64_t>(kUnixUsPerSecond));
  return true;
#else
  (void)source;
  return false;
#endif
}
