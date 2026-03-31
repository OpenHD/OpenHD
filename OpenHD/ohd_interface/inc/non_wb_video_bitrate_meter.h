#ifndef OPENHD_NON_WB_VIDEO_BITRATE_METER_H
#define OPENHD_NON_WB_VIDEO_BITRATE_METER_H

#include <array>
#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>

namespace openhd::non_wb {

struct VideoRateSample {
  int32_t bitrate_bps = 0;
  int32_t packets_per_second = 0;
  uint64_t total_packets = 0;
};

class VideoBitrateMeter {
 public:
  static constexpr int kMaxStreams = 2;

  void on_tx_fragment(int stream_index, uint64_t n_bytes) {
    if (stream_index < 0 || stream_index >= kMaxStreams) {
      return;
    }
    if (n_bytes == 0) {
      return;
    }
    m_total_bytes.at(stream_index).fetch_add(n_bytes, std::memory_order_relaxed);
    m_total_packets.at(stream_index).fetch_add(1, std::memory_order_relaxed);
  }

  VideoRateSample sample_stream(int stream_index, int64_t now_ms) {
    VideoRateSample ret{};
    if (stream_index < 0 || stream_index >= kMaxStreams) {
      return ret;
    }
    std::lock_guard<std::mutex> guard(m_mutex);
    const auto idx = static_cast<size_t>(stream_index);
    const uint64_t total_bytes = m_total_bytes.at(idx).load(std::memory_order_relaxed);
    const uint64_t total_packets =
        m_total_packets.at(idx).load(std::memory_order_relaxed);
    ret.total_packets = total_packets;
    if (m_last_ts_ms.at(idx) == 0) {
      m_last_ts_ms.at(idx) = now_ms;
      m_last_bytes.at(idx) = total_bytes;
      m_last_packets.at(idx) = total_packets;
      return ret;
    }
    const int64_t dt_ms = now_ms - m_last_ts_ms.at(idx);
    if (dt_ms <= 0) {
      ret.bitrate_bps = m_last_bps.at(idx);
      ret.packets_per_second = m_last_pps.at(idx);
      return ret;
    }
    const uint64_t delta_bytes =
        total_bytes >= m_last_bytes.at(idx) ? total_bytes - m_last_bytes.at(idx)
                                            : 0;
    const uint64_t delta_packets =
        total_packets >= m_last_packets.at(idx)
            ? total_packets - m_last_packets.at(idx)
            : 0;
    const int64_t bps_i64 =
        static_cast<int64_t>((delta_bytes * 8ULL * 1000ULL) / dt_ms);
    const int64_t pps_i64 =
        static_cast<int64_t>((delta_packets * 1000ULL) / dt_ms);
    ret.bitrate_bps = clamp_int32(bps_i64);
    ret.packets_per_second = clamp_int32(pps_i64);
    m_last_ts_ms.at(idx) = now_ms;
    m_last_bytes.at(idx) = total_bytes;
    m_last_packets.at(idx) = total_packets;
    m_last_bps.at(idx) = ret.bitrate_bps;
    m_last_pps.at(idx) = ret.packets_per_second;
    return ret;
  }

 private:
  static int32_t clamp_int32(int64_t value) {
    if (value > std::numeric_limits<int32_t>::max()) {
      return std::numeric_limits<int32_t>::max();
    }
    if (value < std::numeric_limits<int32_t>::min()) {
      return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(value);
  }

 private:
  std::array<std::atomic<uint64_t>, kMaxStreams> m_total_bytes{};
  std::array<std::atomic<uint64_t>, kMaxStreams> m_total_packets{};
  std::array<int64_t, kMaxStreams> m_last_ts_ms{0, 0};
  std::array<uint64_t, kMaxStreams> m_last_bytes{0, 0};
  std::array<uint64_t, kMaxStreams> m_last_packets{0, 0};
  std::array<int32_t, kMaxStreams> m_last_bps{0, 0};
  std::array<int32_t, kMaxStreams> m_last_pps{0, 0};
  std::mutex m_mutex;
};

}  // namespace openhd::non_wb

#endif  // OPENHD_NON_WB_VIDEO_BITRATE_METER_H
