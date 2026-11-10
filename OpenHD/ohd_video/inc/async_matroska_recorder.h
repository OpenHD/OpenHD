#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "matroska_recorder.h"

// File-system work has its own owner. A slow card cannot hold an encoder input.
// Exhaustion rejects the whole recording session; encoded reference frames are
// never silently dropped from a continuing file.
class AsyncMatroskaRecorder {
 public:
  using Clock = std::chrono::steady_clock;
  struct Packet {
    std::vector<uint8_t> data, header;
    uint64_t timestamp_ms = 0;
    bool keyframe = false;
    Clock::time_point captured, queued = Clock::now();
  };
  struct Completion {
    size_t bytes;
    bool keyframe;
    Clock::time_point captured, queued, write_begin, finished;
  };
  explicit AsyncMatroskaRecorder(size_t capacity = 32 * 1024 * 1024)
      : capacity_(capacity) {}
  ~AsyncMatroskaRecorder() { close(); }
  void open(std::function<std::string()> filename, bool h265, int width,
            int height, int fps,
            std::function<void(const std::string&)> opened,
            std::function<void(const Completion&)> written) {
    close();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = false;
      error_.clear();
      good_ = true;
    }
    thread_ = std::thread([=]() {
      MatroskaRecorder recorder;
      try {
        const auto path = filename();
        if (!recorder.open(path, h265, width, height, fps))
          throw std::runtime_error("Cannot open recording file " + path);
        if (opened) opened(path);
        while (true) {
          Packet packet;
          {
            std::unique_lock<std::mutex> lock(mutex_);
            wake_.wait(lock, [this]() { return stopping_ || !queue_.empty(); });
            if (queue_.empty()) break;
            packet = std::move(queue_.front());
            queue_.pop_front();
          }
          const auto begin = Clock::now();
          if (!packet.header.empty())
            recorder.feed_annex_b_access_unit(packet.header.data(),
                packet.header.size(), packet.timestamp_ms);
          recorder.feed_annex_b_access_unit(packet.data.data(),
              packet.data.size(), packet.timestamp_ms);
          if (!recorder.good()) throw std::runtime_error("Recording write failed");
          if (written) written({packet.data.size(), packet.keyframe,
              packet.captured, packet.queued, begin, Clock::now()});
          {
            std::lock_guard<std::mutex> lock(mutex_);
            bytes_ -= packet.data.size() + packet.header.size();
          }
        }
      } catch (const std::exception& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = error.what();
        good_ = false;
      } catch (...) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = "Unknown recording writer failure";
        good_ = false;
      }
      recorder.close();
      std::lock_guard<std::mutex> lock(mutex_);
      if (!recorder.good() && error_.empty()) {
        error_ = "Recording flush/close failed";
        good_ = false;
      }
      queue_.clear();
      bytes_ = 0;
    });
  }
  bool enqueue(Packet packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto size = packet.data.size() + packet.header.size();
    if (!good_ || stopping_ || size > capacity_ || bytes_ > capacity_ - size)
      return false;
    bytes_ += size;
    queue_.push_back(std::move(packet));
    wake_.notify_one();
    return true;
  }
  bool good() const { return good_; }
  std::string error() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return error_;
  }
  void close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
      wake_.notify_all();
    }
    if (thread_.joinable()) thread_.join();
    good_ = false;
  }
 private:
  const size_t capacity_;
  size_t bytes_ = 0;
  mutable std::mutex mutex_;
  std::condition_variable wake_;
  std::deque<Packet> queue_;
  std::thread thread_;
  std::atomic<bool> good_{false};
  bool stopping_ = true;
  std::string error_;
};
