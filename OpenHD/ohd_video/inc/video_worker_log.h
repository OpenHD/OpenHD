#pragma once

#include <spdlog/sinks/base_sink.h>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>

namespace openhd::video_worker {
// Never write to disk or the video socket from an encoder/logging thread.
// The control thread drains a bounded queue; congestion sacrifices diagnostics.
class DiagnosticSink : public spdlog::sinks::base_sink<std::mutex> {
 public:
  struct Entry { uint32_t level; std::string text; };
  bool pop(Entry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (entries.empty()) return false;
    entry = std::move(entries.front());
    entries.pop_front();
    return true;
  }
  void dropped() {
    std::lock_guard<std::mutex> lock(mutex_);
    ++lost;
  }
  uint64_t take_lost() {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto result = lost;
    lost = 0;
    return result;
  }
 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    if (msg.level < spdlog::level::info) return;
    if (entries.size() >= 128) { ++lost; return; }
    std::string text(msg.logger_name.data(), msg.logger_name.size());
    text += ": ";
    text.append(msg.payload.data(), msg.payload.size());
    text.resize(std::min<size_t>(text.size(), 2048));
    entries.push_back({static_cast<uint32_t>(msg.level), std::move(text)});
  }
  void flush_() override {}
 private:
  std::deque<Entry> entries;
  uint64_t lost = 0;
};
}  // namespace openhd::video_worker
