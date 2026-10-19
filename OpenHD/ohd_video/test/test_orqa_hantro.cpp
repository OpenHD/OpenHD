#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "orqa_hantro_encoder.h"

// On-device regression: stop the camera owner first. Optional output is an
// Annex-B elementary stream for an independent decoder check.
int main(int argc, char** argv) {
  OrqaHantroEncoder::Config config;
  config.device = "/dev/video2";
  int seconds = 32;
  const char* output_path = nullptr;
  bool repeat_rate = false;
  unsigned next_bitrate = 3000;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--h265")
      config.h265 = true;
    else if (arg == "--output" && i + 1 < argc)
      output_path = argv[++i];
    else if (arg == "--seconds" && i + 1 < argc)
      seconds = std::atoi(argv[++i]);
    else if (arg == "--device" && i + 1 < argc)
      config.device = argv[++i];
    else if (arg == "--width" && i + 1 < argc)
      config.width = std::atoi(argv[++i]);
    else if (arg == "--height" && i + 1 < argc)
      config.height = std::atoi(argv[++i]);
    else if (arg == "--fps" && i + 1 < argc)
      config.fps = std::atoi(argv[++i]);
    else if (arg == "--repeat-rate")
      repeat_rate = true;
    else if (arg == "--bitrate" && i + 1 < argc)
      config.bitrate_kbits = std::atoi(argv[++i]);
    else if (arg == "--next-bitrate" && i + 1 < argc)
      next_bitrate = std::atoi(argv[++i]);
    else if (arg == "--keyint" && i + 1 < argc)
      config.keyframe_interval = std::atoi(argv[++i]);
    else
      return 2;
  }
  if (seconds < 12 || next_bitrate < 10) return 2;
  FILE* file = output_path ? std::fopen(output_path, "wb") : nullptr;
  if (output_path && !file) return 2;
  uint64_t keyframes = 0, before_change = 0, after_change = 0;
  uint64_t output_frames = 0;
  double maximum_gap_ms = 0;
  std::chrono::steady_clock::time_point last_output;
  bool changed = false, forced = false, write_ok = true;
  bool measuring = false;
  OrqaHantroEncoder encoder(
      config,
      [&](const uint8_t* data, size_t bytes, bool keyframe, int64_t capture_us) {
        if (file && std::fwrite(data, 1, bytes, file) != bytes)
          write_ok = false;
        if (keyframe) ++keyframes;
        if (capture_us > 0 && bytes > 0) {
          const auto now = std::chrono::steady_clock::now();
          if (output_frames)
            maximum_gap_ms = std::max(maximum_gap_ms,
                std::chrono::duration<double, std::milli>(now - last_output).count());
          last_output = now;
          ++output_frames;
        }
        if (measuring) (changed ? after_change : before_change) += bytes;
      },
      [](const std::string& message) { std::cerr << message << '\n'; });
  if (!encoder.start()) {
    if (file) std::fclose(file);
    return 1;
  }
  const auto started = std::chrono::steady_clock::now();
  auto changed_at = started;
  auto reported_at = started, repeated_at = started;
  uint64_t reported_bytes = 0;
  uint64_t pictures = 0, previous_frames = 0;
  uint64_t high_frames = 0, low_frames = 0;
  auto measured_at = started;
  double high_seconds = 0, low_seconds = 0;
  bool ok = true;
  while (std::chrono::steady_clock::now() - started <
         std::chrono::seconds(seconds)) {
    if (!encoder.process()) {
      ok = false;
      break;
    }
    const auto elapsed = std::chrono::steady_clock::now() - started;
    const auto now = std::chrono::steady_clock::now();
    if (repeat_rate && now - repeated_at >= std::chrono::milliseconds(100)) {
      if (!encoder.set_rate(changed ? next_bitrate : config.bitrate_kbits,
                            config.qp_min, config.qp_max)) {
        ok = false;
        break;
      }
      repeated_at = now;
    }
    if (now - reported_at >= std::chrono::seconds(1)) {
      const auto bytes = encoder.stats().bytes;
      const auto frames = encoder.stats().frames;
      std::cout << "target_kbps=" << (changed ? next_bitrate : config.bitrate_kbits)
                << " actual_kbps=" << (bytes - reported_bytes) * 8 /
                    std::chrono::duration<double>(now - reported_at).count() / 1000
                << " fps=" << (frames - previous_frames) /
                    std::chrono::duration<double>(now - reported_at).count()
                << std::endl;
      reported_at = now;
      reported_bytes = bytes;
      previous_frames = frames;
    }
    if (!measuring && now - changed_at >= std::chrono::seconds(2)) {
      measuring = true;
      measured_at = now;
      pictures = encoder.stats().frames;
    }
    if (!changed && elapsed >= std::chrono::seconds(seconds / 2)) {
      high_seconds = std::chrono::duration<double>(now - measured_at).count();
      high_frames = encoder.stats().frames - pictures;
      measuring = false;
      changed = true;
      changed_at = std::chrono::steady_clock::now();
      if (!encoder.set_rate(next_bitrate, config.qp_min, config.qp_max)) {
        ok = false;
        break;
      }
    }
    if (!forced && elapsed >= std::chrono::seconds(1)) {
      encoder.request_keyframe();
      forced = true;
    }
  }
  if (file) std::fclose(file);
  const auto& stats = encoder.stats();
  const auto finished = std::chrono::steady_clock::now();
  low_seconds = std::chrono::duration<double>(finished - measured_at).count();
  low_frames = stats.frames - pictures;
  const uint64_t high_bps =
      high_seconds > 0 ? before_change * 8 / high_seconds : 0;
  const uint64_t low_bps = low_seconds > 0 ? after_change * 8 / low_seconds : 0;
  std::cout << "frames=" << stats.frames << " dropped=" << stats.dropped
            << " bytes=" << stats.bytes << " keyframes=" << keyframes
            << " before/after=" << before_change << '/' << after_change
            << " high/low_bps=" << high_bps << '/' << low_bps
            << " output_frames=" << output_frames
            << " max_gap_ms=" << maximum_gap_ms
            << " measured_fps=" << high_frames / high_seconds << '/'
            << low_frames / low_seconds
            << " encode_avg/max_us="
            << (stats.frames ? stats.encode_us / stats.frames : 0) << '/'
            << stats.max_encode_us << " capture_to_output_avg/max_us="
            << (stats.frames ? stats.capture_to_output_us / stats.frames : 0)
            << '/' << stats.max_capture_to_output_us << '\n';
  const auto close_to_target = [](uint64_t actual, uint64_t target) {
    return actual >= target * 9 / 10 && actual <= target * 11 / 10;
  };
  const bool rate_ok = close_to_target(high_bps, config.bitrate_kbits * 1000ULL) &&
                       close_to_target(low_bps, next_bitrate * 1000ULL);
  return ok && write_ok && rate_ok && output_frames == stats.frames &&
                 maximum_gap_ms < 250 &&
                 stats.frames >=
                     static_cast<uint64_t>(seconds * config.fps * 8 / 10) &&
                 keyframes > 1 && changed && after_change > 0
             ? 0
             : 1;
}
