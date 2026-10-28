#pragma once
#include <algorithm>
#include <cstdint>
#include <deque>
#include <limits>

namespace openhd::ethernet {
constexpr uint32_t kUnknown = UINT32_MAX;
struct Metrics {
  uint32_t tx_bps = 0, rx_bps = 0, tx_pps = 0, rx_pps = 0;
  uint32_t rtt_us = kUnknown, jitter_us = kUnknown;
  uint32_t probe_loss_permille = kUnknown, send_errors = 0;
};
inline uint32_t rate(uint64_t delta, uint64_t elapsed_ms, uint32_t scale) {
  if (!elapsed_ms) return 0;
  return static_cast<uint32_t>(std::min<long double>(
      UINT32_MAX, static_cast<long double>(delta) * scale / elapsed_ms));
}
// A bounded window of completed probes. Unanswered probes are only counted
// after their deadline. Peer changes reset the window; unsupported peers stay
// unknown.
class ProbeWindow {
 public:
  void reset() { *this = ProbeWindow{}; }
  void complete(bool received, uint32_t rtt_us = 0) {
    results.push_back(received);
    if (results.size() > 60) results.pop_front();
    if (!received) return;
    if (last_rtt != kUnknown) {
      const uint32_t difference =
          last_rtt > rtt_us ? last_rtt - rtt_us : rtt_us - last_rtt;
      jitter = jitter == kUnknown ? difference
                                  : (uint64_t(jitter) * 3 + difference) / 4;
    } else
      jitter = 0;
    last_rtt = rtt_us;
    supported = true;
  }
  void apply(Metrics& metrics, bool fresh) const {
    if (!supported) return;
    metrics.probe_loss_permille =
        results.empty()
            ? kUnknown
            : 1000 * std::count(results.begin(), results.end(), false) /
                  results.size();
    if (fresh) {
      metrics.rtt_us = last_rtt;
      metrics.jitter_us = jitter;
    }
  }

 private:
  std::deque<bool> results;
  uint32_t last_rtt = kUnknown, jitter = kUnknown;
  bool supported = false;
};
}  // namespace openhd::ethernet
