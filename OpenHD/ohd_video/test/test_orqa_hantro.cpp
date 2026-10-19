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
  int seconds = 16;
  const char* output_path = nullptr;
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
    else
      return 2;
  }
  if (seconds < 8) return 2;
  FILE* file = output_path ? std::fopen(output_path, "wb") : nullptr;
  if (output_path && !file) return 2;
  uint64_t keyframes = 0, before_change = 0, after_change = 0;
  bool changed = false, forced = false, write_ok = true;
  OrqaHantroEncoder encoder(
      config,
      [&](const uint8_t* data, size_t bytes, bool keyframe, int64_t) {
        if (file && std::fwrite(data, 1, bytes, file) != bytes)
          write_ok = false;
        if (keyframe) ++keyframes;
        (changed ? after_change : before_change) += bytes;
      },
      [](const std::string& message) { std::cerr << message << '\n'; });
  if (!encoder.start()) {
    if (file) std::fclose(file);
    return 1;
  }
  const auto started = std::chrono::steady_clock::now();
  auto changed_at = started;
  bool ok = true;
  while (std::chrono::steady_clock::now() - started <
         std::chrono::seconds(seconds)) {
    if (!encoder.process()) {
      ok = false;
      break;
    }
    const auto elapsed = std::chrono::steady_clock::now() - started;
    if (!changed && elapsed >= std::chrono::seconds(seconds / 2)) {
      changed = true;
      changed_at = std::chrono::steady_clock::now();
      if (!encoder.set_rate(3000, 8, 51)) {
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
  const double high_seconds =
      std::chrono::duration<double>(changed_at - started).count();
  const double low_seconds =
      std::chrono::duration<double>(finished - changed_at).count();
  const uint64_t high_bps =
      high_seconds > 0 ? before_change * 8 / high_seconds : 0;
  const uint64_t low_bps = low_seconds > 0 ? after_change * 8 / low_seconds : 0;
  std::cout << "frames=" << stats.frames << " dropped=" << stats.dropped
            << " bytes=" << stats.bytes << " keyframes=" << keyframes
            << " before/after=" << before_change << '/' << after_change
            << " high/low_bps=" << high_bps << '/' << low_bps
            << " encode_avg/max_us="
            << (stats.frames ? stats.encode_us / stats.frames : 0) << '/'
            << stats.max_encode_us << " capture_to_output_avg/max_us="
            << (stats.frames ? stats.capture_to_output_us / stats.frames : 0)
            << '/' << stats.max_capture_to_output_us << '\n';
  const bool rate_ok = high_bps >= 5600000 && high_bps <= 10400000 &&
                       low_bps >= 2100000 && low_bps <= 3900000;
  return ok && write_ok && rate_ok &&
                 stats.frames >=
                     static_cast<uint64_t>(seconds * config.fps * 8 / 10) &&
                 keyframes > 1 && changed && after_change > 0
             ? 0
             : 1;
}
