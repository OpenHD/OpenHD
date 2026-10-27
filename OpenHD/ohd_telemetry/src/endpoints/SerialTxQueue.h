// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <algorithm>
#include <chrono>
#include <deque>
#include <optional>

#include "../mav_include.h"

// Caller owns the mutex. Limits apply to complete frames, never byte fragments.
class SerialTxQueue {
 public:
  using Clock = std::chrono::steady_clock;
  static constexpr size_t capacity = 128;
  struct Item {
    MavlinkMessage message;
    Clock::time_point queued;
    int priority;
    uint64_t generation;
  };
  static bool is_rc(const MavlinkMessage& message) {
    return message.m.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE ||
           message.m.msgid == MAVLINK_MSG_ID_MANUAL_CONTROL;
  }
  static int traffic_priority(const MavlinkMessage& message) {
    switch (message.m.msgid) {
      case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
      case MAVLINK_MSG_ID_MANUAL_CONTROL: return 3;
      case MAVLINK_MSG_ID_COMMAND_LONG:
      case MAVLINK_MSG_ID_COMMAND_INT:
      case MAVLINK_MSG_ID_COMMAND_ACK:
      case MAVLINK_MSG_ID_HEARTBEAT:
      case MAVLINK_MSG_ID_TIMESYNC: return 2;
      case MAVLINK_MSG_ID_PARAM_VALUE:
      case MAVLINK_MSG_ID_PARAM_EXT_VALUE:
      case MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL: return 0;
      default: return 1;
    }
  }
  static int priority(const MavlinkMessage& message) {
    return (message.uart_priority >= 0 ? std::min(10, message.uart_priority) * 4 : 0)
           + traffic_priority(message);
  }
  bool push(const MavlinkMessage& message, Clock::time_point now = Clock::now(),
            uint64_t generation = 0) {
    expire(now);
    const int rank = priority(message);
    // Only unsent RC state may be replaced; transactions remain distinct.
    if (is_rc(message) && !(message.m.incompat_flags & MAVLINK_IFLAG_SIGNED)) {
      const auto old = std::find_if(items.begin(), items.end(), [&](const Item& item) {
        return item.message.m.msgid == message.m.msgid &&
               item.message.m.sysid == message.m.sysid &&
               item.message.m.compid == message.m.compid &&
               !(item.message.m.incompat_flags & MAVLINK_IFLAG_SIGNED);
      });
      if (old != items.end()) { items.erase(old); ++replaced; }
    }
    if (items.size() == capacity) {
      const auto lowest = std::min_element(items.begin(), items.end(),
          [](const Item& a, const Item& b) { return a.priority < b.priority; });
      ++dropped;
      if (lowest->priority >= rank) return false;
      items.erase(lowest);
    }
    items.push_back({message, now, rank, generation});
    high_water = std::max(high_water, items.size());
    return true;
  }
  std::optional<Item> pop(Clock::time_point now = Clock::now()) {
    expire(now);
    if (items.empty()) return std::nullopt;
    // Allow bulk/normal traffic to make progress even under sustained control.
    auto chosen = items.begin();
    if (priority_burst < 8) {
      chosen = std::max_element(items.begin(), items.end(),
          [](const Item& a, const Item& b) { return a.priority < b.priority; });
      ++priority_burst;
    } else { priority_burst = 0; }
    // Signed MAVLink uses monotonic timestamps. Preserve signed frame order
    // within a sender even when a later command has a higher traffic priority.
    if (chosen->message.m.incompat_flags & MAVLINK_IFLAG_SIGNED) {
      for (auto i = items.begin(); i != chosen; ++i) {
        if ((i->message.m.incompat_flags & MAVLINK_IFLAG_SIGNED) &&
            i->message.m.sysid == chosen->message.m.sysid &&
            i->message.m.compid == chosen->message.m.compid) {
          chosen = i; break;
        }
      }
    }
    auto result = *chosen;
    items.erase(chosen);
    return result;
  }
  void clear() { dropped += items.size(); items.clear(); priority_burst = 0; }
  size_t size() const { return items.size(); }
  uint64_t dropped = 0, expired = 0, replaced = 0;
  size_t high_water = 0;

 private:
  void expire(Clock::time_point now) {
    for (auto i = items.begin(); i != items.end();) {
      const auto ttl = is_rc(i->message) ? std::chrono::milliseconds(250)
                                      : std::chrono::milliseconds(5000);
      if (now - i->queued > ttl) { i = items.erase(i); ++expired; }
      else ++i;
    }
  }
  std::deque<Item> items;
  unsigned priority_burst = 0;
};
