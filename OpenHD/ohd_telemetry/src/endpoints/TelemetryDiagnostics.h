// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <map>
#include <mutex>
#include <sstream>
#include <tuple>

#include "../mav_include.h"
#include "openhd_spdlog_include.h"

// Always-on, bounded, payload-free counters in the normal exported logs.
// These describe endpoint acceptance/parsing, not delivery to a remote peer.
class TelemetryDiagnostics {
 public:
  enum Event { Rx, TxAccepted, TxRejected, BadCrc, BadSignature };
  void record(const mavlink_message_t& message, Event event) {
    std::lock_guard<std::mutex> lock(mutex);
    const auto key = std::make_tuple(message.sysid, message.compid, message.msgid);
    auto found = counts.find(key);
    if (found == counts.end() && counts.size() >= 128) { ++untracked; return; }
    ++counts[key].events[event];
    if (event == Rx && message.msgid == MAVLINK_MSG_ID_OPENHD_STATS_MONITOR_MODE_WIFI_CARD) {
      mavlink_openhd_stats_monitor_mode_wifi_card_t card{};
      mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&message, &card);
      auto& row = counts[key];
      row.last_card = card.card_index; row.last_rssi = card.rx_rssi;
      row.last_card_time = std::chrono::steady_clock::now();
    }
  }
  void callback_time(std::chrono::steady_clock::duration duration) {
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::lock_guard<std::mutex> lock(mutex);
    max_callback_ms = std::max<int64_t>(max_callback_ms, ms);
  }
  void log_if_due(const std::string& endpoint) {
    const auto now = std::chrono::steady_clock::now();
    std::string summary;
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (now - last_log < std::chrono::seconds(5)) return;
      last_log = now;
      std::ostringstream out;
      out << "telemetry-path-v1 endpoint=" << endpoint
          << " callback_max_ms=" << max_callback_ms << " untracked=" << untracked
          << " counts=sys/comp/id:rx,tx_batch_ok,tx_batch_failed,bad_crc,bad_signature";
      for (const auto& entry : counts) {
        out << ' ' << unsigned(std::get<0>(entry.first)) << '/'
            << unsigned(std::get<1>(entry.first)) << '/' << std::get<2>(entry.first) << ':';
        const auto& row = entry.second;
        for (int i = 0; i < 5; ++i) out << (i ? "," : "") << row.events[i];
        if (row.last_card >= 0) {
          out << "(card=" << row.last_card << ",rssi=" << row.last_rssi
              << ",age_ms=" << std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - row.last_card_time).count() << ')';
        }
      }
      summary = out.str();
      max_callback_ms = 0;
    }
    openhd::log::get_default()->info(summary);
  }

 private:
  struct Counts {
    uint64_t events[5]{};
    int last_card = -1, last_rssi = -128;
    std::chrono::steady_clock::time_point last_card_time{};
  };
  std::mutex mutex;
  std::map<std::tuple<uint8_t, uint8_t, uint32_t>, Counts> counts;
  uint64_t untracked = 0;
  int64_t max_callback_ms = 0;
  std::chrono::steady_clock::time_point last_log = std::chrono::steady_clock::now();
};
