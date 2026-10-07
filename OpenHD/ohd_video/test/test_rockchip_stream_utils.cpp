#include <atomic>
#include <future>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

#include "../src/rockchip_stream_utils.h"

using namespace std::chrono_literals;
using namespace openhd::mpp;

void check(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

int main() {
  Nv12Layout padded{32, 32 * 16};
  check(padded.fits(32 * 16 * 3 / 2, 16, 10), "padded NV12 rejected");
  check(padded.matches(32, 16), "compatible DMA layout rejected");
  check(!padded.matches(16, 16), "incompatible DMA stride accepted");
  check(!padded.matches(32, 10), "incompatible DMA UV offset accepted");
  check(!padded.fits(512, 16, 10), "truncated chroma accepted");
  check(!padded.fits(768, 33, 10), "invalid width accepted");
  check(!Nv12Layout{0, 0}.matches(0, 0), "zero stride accepted");
  check(!Nv12Layout{std::numeric_limits<size_t>::max(), 0}.fits(768, 16, 10),
        "overflowing layout accepted");

  CaptureSequence sequence;
  check(sequence.observe(0xfffffffeU) == 0, "first sequence has drops");
  check(sequence.observe(1) == 2, "wrap gap not counted");
  check(sequence.observe(0) == 0, "reset counted as a huge gap");
  check(sequence.observe(0) == 0, "duplicate counted as gap");
  check(sequence.observe(3) == 2, "ordinary gap missing");
  sequence.reset();
  check(sequence.observe(99) == 0, "restart retained old sequence");

  LatencySamples latency;
  for (int i = 1; i <= 100; ++i) latency.add(i);
  auto samples = latency.take();
  check(LatencySamples::percentile(samples, 50) == 50, "p50 wrong");
  check(LatencySamples::percentile(samples, 95) == 95, "p95 wrong");
  check(latency.take().empty(), "window not cleared");
  for (int i = 0; i < 1000; ++i) latency.add(i);
  samples = latency.take();
  check(
      samples.size() == 512 && samples.front() == 488 && samples.back() == 999,
      "sample storage not bounded to latest observations");

  SlotQueue<int, 3> queue;
  auto a = queue.reserve(), b = queue.reserve(), c = queue.reserve();
  check(a && b && c && !queue.reserve(), "pool overcommitted");
  check(queue.publish(*a, 11) && queue.publish(*b, 22), "publish failed");
  auto active = queue.wait(0ms);
  check(active && active->first == *a && active->second == 11, "FIFO broken");
  queue.discard_pending();
  auto freed = queue.reserve();
  check(freed && *freed == *b && !queue.reserve(),
        "discard reused an active or producer-owned buffer");
  queue.release(*freed);
  queue.release(*c);
  queue.release(active->first);
  auto next = queue.reserve();
  check(next && queue.publish(*next, 33), "recycled slot unusable");
  queue.stop();
  check(!queue.wait(0ms) && !queue.reserve(), "stop retained queued work");
  queue.reset();
  auto waiter = std::async(std::launch::async, [&] { return queue.wait(5s); });
  queue.stop();
  check(waiter.wait_for(500ms) == std::future_status::ready && !waiter.get(),
        "stop did not wake consumer");

  // Deliberately stall the consumer. Producers must remain nonblocking and
  // may never overwrite a buffer still held by the consumer.
  queue.reset();
  auto held = queue.reserve();
  queue.publish(*held, 42);
  active = queue.wait(0ms);
  const auto begin = std::chrono::steady_clock::now();
  for (int i = 0; i < 10000; ++i) {
    auto slot = queue.reserve();
    if (slot) {
      check(*slot != active->first,
            "producer reused stalled consumer's buffer");
      queue.publish(*slot, i);
    }
  }
  check(std::chrono::steady_clock::now() - begin < 500ms,
        "producer waited on stalled consumer");
  queue.stop();
  queue.release(active->first);
  std::cout << "MPP layout, sequence, latency, pool ownership and overload "
               "checks passed\n";
}
