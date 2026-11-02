#pragma once
#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include "openhd_bandwidth_allocation.hpp"

namespace openhd::bandwidth {
// Capacity is supplied by the transport, never inferred from the bitrate of
// the stream we are currently sending. Independent renditions own independent
// controllers; a replicated rendition must fit every routed video transport.
class RateController {
 public:
  int update(int ceiling_kbits, int capacity_kbits, uint32_t rtt_us,
             uint32_t loss_permille, uint32_t errors, int64_t now_ms,
             bool queue_congested = false) {
    const int ceiling = std::max(64, ceiling_kbits);
    const int limit = capacity_kbits > 0
        ? std::max(64, std::min(ceiling, capacity_kbits * 8 / 10)) : ceiling;
    const bool has_rtt = rtt_us != UINT32_MAX;
    while (!rtt_history_.empty() && now_ms - rtt_history_.front().first > 30000) rtt_history_.pop_front();
    if (has_rtt) rtt_history_.emplace_back(now_ms, rtt_us);
    baseline_rtt_ = 0;
    for (const auto& sample : rtt_history_)
      if (!baseline_rtt_ || sample.second < baseline_rtt_) baseline_rtt_ = sample.second;
    const bool delayed = has_rtt && baseline_rtt_ &&
        rtt_us > std::max<uint64_t>(uint64_t(baseline_rtt_) * 2, uint64_t(baseline_rtt_) + 25000);
    const bool congested = queue_congested || delayed ||
        (loss_permille != UINT32_MAX && loss_permille >= 50) || errors > last_errors_;
    last_errors_ = errors;
    if (!target_) target_ = capacity_kbits > 0 ? limit : std::min(limit, 1000);
    target_ = std::min(target_, limit);  // Capacity reductions are immediate.
    if (congested) {
      healthy_since_ = now_ms;
      if (last_reduction_ < 0 || now_ms - last_reduction_ >= 1000) {
        target_ = std::max(std::min(256, limit), target_ * 8 / 10);
        last_reduction_ = now_ms;
      }
    } else if ((has_rtt || capacity_kbits > 0) && now_ms - healthy_since_ >= 3000 &&
               now_ms - last_increase_ >= 1000) {
      target_ = std::min(limit, target_ + std::max(100, target_ / 20));
      last_increase_ = now_ms;
    }
    return target_;
  }
 private:
  int target_ = 0;
  uint32_t baseline_rtt_ = 0, last_errors_ = 0;
  std::deque<std::pair<int64_t, uint32_t>> rtt_history_;
  int64_t healthy_since_ = 0, last_increase_ = 0, last_reduction_ = -1;
};

struct Recommendation {
  Transport transport = Transport::Unknown;
  int kbits = 0;
};
class RelayConstraint {
 public:
  void update(int kbits, int64_t now_ms) { limit_ = std::clamp(kbits, 64, 40000); updated_ = now_ms; }
  int constrain(int kbits, int64_t now_ms) const {
    return limit_ > 0 && now_ms - updated_ < 3000 ? std::min(kbits, limit_) : kbits;
  }
 private:
  int limit_ = 0;
  int64_t updated_ = 0;
};
class BudgetRouter {
 public:
  void routed(Transport transport, bool enabled) {
    if (transport == Transport::Unknown) return;
    if (enabled) active_.insert(transport);
    else { active_.erase(transport); budgets_.erase(transport); }
  }
  std::optional<Recommendation> update(Transport transport, int kbits) {
    if (!active_.count(transport) || kbits <= 0) return std::nullopt;
    budgets_[transport] = kbits;
    std::optional<Recommendation> result;
    for (const auto& entry : budgets_) {
      if (!active_.count(entry.first)) continue;
      if (!result || entry.second < result->kbits) result = Recommendation{entry.first, entry.second};
    }
    return result;
  }
 private:
  std::set<Transport> active_;
  std::map<Transport, int> budgets_;
};
inline Transport transport_for(const std::string& name) {
  if (name == "WIFIBROADCAST") return Transport::WiFiBroadcast;
  if (name == "ETHERNET") return Transport::Ethernet;
  if (name == "MICROHARD") return Transport::Microhard;
  if (name == "ARTOSYN") return Transport::Artosyn;
  if (name == "LTE" || name == "FLEETCONTROL") return Transport::Lte;
  return Transport::Unknown;
}
inline const char* transport_name(Transport transport) {
  switch (transport) {
    case Transport::WiFiBroadcast: return "WIFIBROADCAST";
    case Transport::Ethernet: return "ETHERNET";
    case Transport::Microhard: return "MICROHARD";
    case Transport::Artosyn: return "ARTOSYN";
    case Transport::Lte: return "LTE";
    default: return "UNKNOWN";
  }
}
}  // namespace openhd::bandwidth
