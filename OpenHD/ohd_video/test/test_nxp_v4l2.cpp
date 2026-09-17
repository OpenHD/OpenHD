#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "camera_enums.hpp"
#include "camera_holder.h"
#include "nxp_v4l2_stream.h"
#include "openhd_udp.h"

namespace {
std::atomic_bool keep_running{true};
void stop_on_signal(int) { keep_running = false; }
}  // namespace

int main(int argc, char* argv[]) {
  using Clock = std::chrono::steady_clock;
  XCamera camera{};
  camera.camera_type = X_CAM_TYPE_ORQA_REKINDLE;
  camera.index = 0;
  camera.usb_v4l2_device_number = -1;
  auto holder = std::make_shared<CameraHolder>(camera);
  bool use_h265 = false;
  std::string udp_target;
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--h265") {
      use_h265 = true;
    } else if (argument == "--udp" && i + 1 < argc) {
      udp_target = argv[++i];
    } else {
      std::cerr << "Usage: " << argv[0] << " [--h265] [--udp target-ip]\n";
      return 2;
    }
  }
  if (!holder->set_video_codec(use_h265 ? 1 : 0)) return 2;
  if (!holder->set_video_bitrate(12)) return 2;

  std::unique_ptr<openhd::UDPForwarder> forwarder;
  if (!udp_target.empty())
    forwarder = std::make_unique<openhd::UDPForwarder>(udp_target, 5600);

  std::atomic<uint64_t> callbacks{0};
  std::atomic<uint64_t> packets{0};
  std::atomic<uint64_t> bytes{0};
  std::atomic<uint64_t> keyframes{0};
  std::atomic<int64_t> last_callback_us{0};
  std::atomic<int64_t> maximum_gap_us{0};
  auto callback = [&](int, const openhd::FragmentedVideoFrame& frame) {
    const auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now().time_since_epoch())
                            .count();
    const auto previous_us = last_callback_us.exchange(now_us);
    if (previous_us > 0) {
      const auto gap_us = now_us - previous_us;
      auto maximum = maximum_gap_us.load();
      while (gap_us > maximum &&
             !maximum_gap_us.compare_exchange_weak(maximum, gap_us)) {
      }
    }
    ++callbacks;
    packets += frame.rtp_fragments.size();
    if (frame.is_idr_frame) ++keyframes;
    for (const auto& fragment : frame.rtp_fragments) {
      bytes += fragment->size();
      if (forwarder)
        forwarder->forwardPacketViaUDP(fragment->data(), fragment->size());
    }
  };

  NxpV4l2Stream stream(holder, callback);
  stream.start_looping();
  if (forwarder) {
    std::signal(SIGINT, stop_on_signal);
    std::signal(SIGTERM, stop_on_signal);
    std::cout << "Streaming " << (use_h265 ? "H265" : "H264")
              << " RTP to " << udp_target
              << ":5600; press Ctrl-C to stop\n";
    while (keep_running) std::this_thread::sleep_for(std::chrono::seconds(1));
    stream.terminate_looping();
    return callbacks > 0 ? 0 : 1;
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));

  const auto high_started = Clock::now();
  const auto high_start_bytes = bytes.load();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  const auto high_finished = Clock::now();
  const auto high_bytes = bytes.load() - high_start_bytes;

  const auto callbacks_before_change = callbacks.load();
  stream.handle_change_bitrate_request({3000});

  // Allow the encoder's rate controller to settle, while continuously checking
  // that the live control update did not stall or restart video.
  for (int i = 0; i < 10; ++i) {
    const auto before = callbacks.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (callbacks.load() == before) {
      std::cerr << "No encoded video during live bitrate transition\n";
      stream.terminate_looping();
      return 1;
    }
  }

  const auto low_started = Clock::now();
  const auto low_start_bytes = bytes.load();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  const auto low_finished = Clock::now();
  const auto low_bytes = bytes.load() - low_start_bytes;
  const auto cam_info = openhd::LinkActionHandler::instance().get_cam_info(0);
  stream.terminate_looping();

  const auto bits_per_second = [](uint64_t byte_count, auto begin, auto end) {
    const auto elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count();
    return byte_count * 8ULL * 1000000ULL /
           static_cast<uint64_t>(std::max<int64_t>(1, elapsed_us));
  };
  const auto high_bps = bits_per_second(high_bytes, high_started, high_finished);
  const auto low_bps = bits_per_second(low_bytes, low_started, low_finished);
  const auto maximum_gap_ms = maximum_gap_us.load() / 1000.0;
  const bool stream_continuous =
      callbacks.load() > callbacks_before_change && maximum_gap_ms < 250.0;
  const auto close_to_target = [](uint64_t measured, uint64_t target) {
    return measured >= target * 7 / 10 && measured <= target * 13 / 10;
  };
  const bool bitrate_changed = close_to_target(high_bps, 12000000) &&
                               close_to_target(low_bps, 3000000) &&
                               low_bps < high_bps * 3 / 4;
  const bool telemetry_valid =
      cam_info.active && cam_info.cam_status == CameraStream::CAM_STATUS_STREAMING &&
      cam_info.encoding_format == static_cast<uint8_t>(use_h265 ? 1 : 0) &&
      cam_info.encoding_bitrate_kbits == 3000 &&
      cam_info.target_bitrate_kbits == 3000 && cam_info.stream_w == 960 &&
      cam_info.stream_h == 720 && cam_info.stream_fps == 60 &&
      cam_info.measured_bitrate_bps > 0 && cam_info.measured_fps > 0 &&
      cam_info.supports_variable_bitrate == 1;

  std::cout << "codec=" << (use_h265 ? "H265" : "H264")
            << " callbacks=" << callbacks << " packets=" << packets
            << " bytes=" << bytes << " keyframes=" << keyframes
            << " high_bps=" << high_bps << " low_bps=" << low_bps
            << " max_gap_ms=" << maximum_gap_ms
            << " telemetry=" << cam_info.stream_w << 'x' << cam_info.stream_h
            << '@' << cam_info.stream_fps
            << " measured=" << cam_info.measured_bitrate_bps << "bps/"
            << cam_info.measured_fps << "fps target="
            << cam_info.target_bitrate_kbits << "kbit/s\n";
  return callbacks > 0 && packets > 0 && bytes > 0 && stream_continuous &&
                 bitrate_changed && telemetry_valid
             ? 0
             : 1;
}
