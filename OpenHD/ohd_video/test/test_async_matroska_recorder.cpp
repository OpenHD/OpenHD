#include <cassert>
#include <cstdio>
#include <future>
#include <iostream>

#include "async_matroska_recorder.h"

int main() {
  using namespace std::chrono_literals;
  std::promise<void> release;
  auto gate = release.get_future().share();
  std::atomic<int> written{0};
  AsyncMatroskaRecorder recorder(8);
  recorder.open([gate] { gate.wait(); return "/tmp/openhd-async-recorder-test.mkv"; },
                false, 128, 96, 10, {},
                [&](const auto&) { ++written; });
  // Filename/counter creation may block on the card; neither producer nor
  // queue-capacity checks may wait for it.
  auto producer = std::async(std::launch::async, [&] {
    AsyncMatroskaRecorder::Packet packet;
    packet.data.resize(8);
    assert(recorder.enqueue(std::move(packet)));
    AsyncMatroskaRecorder::Packet overflow;
    overflow.data.resize(1);
    assert(!recorder.enqueue(std::move(overflow)));
  });
  assert(producer.wait_for(500ms) == std::future_status::ready);
  producer.get();
  release.set_value();
  recorder.close();
  assert(written == 1);  // Close drains accepted encoded data.
  std::remove("/tmp/openhd-async-recorder-test.mkv");

  std::promise<void> failed;
  auto failure = failed.get_future();
  recorder.open([&]() -> std::string {
    failed.set_value();
    throw std::runtime_error("read-only recording counter");
  }, false, 128, 96, 10, {}, {});
  failure.wait();
  recorder.close();
  assert(recorder.error() == "read-only recording counter");
  assert(!recorder.good());
  std::cout << "Asynchronous recording stall, capacity, drain and exception checks passed\n";
}
