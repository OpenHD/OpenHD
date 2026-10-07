#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace openhd::mpp {

// Samples are bounded; percentiles describe the latest 512 observations.
class LatencySamples {
 public:
  void add(double milliseconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    samples_[count_++ % samples_.size()] = milliseconds;
  }

  std::vector<double> take() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<double> result(
        samples_.begin(), samples_.begin() + std::min(count_, samples_.size()));
    count_ = 0;
    std::sort(result.begin(), result.end());
    return result;
  }

  static double percentile(const std::vector<double>& sorted,
                           unsigned percent) {
    if (sorted.empty()) return 0;
    const size_t rank = (sorted.size() * std::min(percent, 100U) + 99) / 100;
    return sorted[rank ? rank - 1 : 0];
  }

 private:
  std::mutex mutex_;
  std::array<double, 512> samples_{};
  size_t count_ = 0;
};

struct Nv12Layout {
  size_t stride;
  size_t uv_offset;

  bool fits(size_t bytes, size_t width, size_t height) const {
    if (!width || !height || width % 2 || height % 2 || stride < width)
      return false;
    // Division avoids overflow on malformed driver metadata.
    return uv_offset <= bytes && height <= uv_offset / stride &&
           height / 2 <= (bytes - uv_offset) / stride;
  }

  bool matches(size_t encoder_stride, size_t encoder_rows) const {
    return stride && stride == encoder_stride &&
           uv_offset / stride == encoder_rows && uv_offset % stride == 0;
  }
};

class CaptureSequence {
 public:
  uint32_t observe(uint32_t sequence) {
    uint32_t missing = 0;
    if (last_) {
      const uint32_t delta = sequence - *last_;
      // Handle wrap, but don't report a driver reset as billions of drops.
      if (delta > 1 && delta < 0x80000000U) missing = delta - 1;
    }
    last_ = sequence;
    return missing;
  }
  void reset() { last_.reset(); }

 private:
  std::optional<uint32_t> last_;
};

// Slots remain reserved while the producer copies or the consumer encodes.
// Discarding queued work never releases a slot still used by either thread.
template <typename Job, size_t Capacity>
class SlotQueue {
 public:
  SlotQueue() { reset(); }

  void reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    free_.clear();
    ready_.clear();
    stopped_ = false;
    for (size_t i = 0; i < Capacity; ++i) free_.push_back(i);
  }

  std::optional<size_t> reserve() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_ || free_.empty()) return {};
    const size_t slot = free_.front();
    free_.pop_front();
    return slot;
  }

  bool publish(size_t slot, Job job) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_) {
      free_.push_back(slot);
      return false;
    }
    ready_.emplace_back(slot, std::move(job));
    wake_.notify_one();
    return true;
  }

  std::optional<std::pair<size_t, Job>> wait(
      std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    wake_.wait_for(lock, timeout,
                   [this] { return stopped_ || !ready_.empty(); });
    if (ready_.empty()) return {};
    auto result = std::move(ready_.front());
    ready_.pop_front();
    return result;
  }

  void release(size_t slot) {
    std::lock_guard<std::mutex> lock(mutex_);
    free_.push_back(slot);
  }

  void discard_pending() {
    std::lock_guard<std::mutex> lock(mutex_);
    discard_locked();
  }

  void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    discard_locked();
    wake_.notify_all();
  }

 private:
  void discard_locked() {
    for (const auto& job : ready_) free_.push_back(job.first);
    ready_.clear();
  }
  std::mutex mutex_;
  std::condition_variable wake_;
  std::deque<size_t> free_;
  std::deque<std::pair<size_t, Job>> ready_;
  bool stopped_ = false;
};

}  // namespace openhd::mpp
