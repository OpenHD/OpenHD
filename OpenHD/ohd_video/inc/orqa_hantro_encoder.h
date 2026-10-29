#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

// Direct VC8000E path. V4L2 capture writes into EWL-allocated encoder input
// buffers; the CPU never copies a raw frame. Only the capture thread may call
// these methods, including live controls and destruction.
class OrqaHantroEncoder {
 public:
  struct Config {
    std::string device;
    unsigned width = 960, height = 720, fps = 60;
    unsigned bitrate_kbits = 8000, keyframe_interval = 5;
    unsigned slices = 0, qp_min = 5, qp_max = 51;
    int rotation = 0;
    int flip = 0;  // OpenHD: none, horizontal, vertical, both (0..3).
    bool h265 = false;
  };
  struct Stats {
    uint64_t frames = 0, dropped = 0, bytes = 0;
    uint64_t encode_us = 0, max_encode_us = 0;
    uint64_t capture_to_output_us = 0, max_capture_to_output_us = 0;
  };
  // Data remains valid only during the callback. Headers are delivered before
  // the first picture. A picture's capture time uses CLOCK_MONOTONIC.
  using Output = std::function<void(const uint8_t*, size_t, bool, int64_t)>;
  using Log = std::function<void(const std::string&)>;
  OrqaHantroEncoder(Config config, Output output, Log log);
  ~OrqaHantroEncoder();
  bool start();
  bool process(int timeout_ms = 100);
  bool set_rate(unsigned bitrate_kbits, unsigned qp_min, unsigned qp_max);
  void request_keyframe();
  const Stats& stats() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
