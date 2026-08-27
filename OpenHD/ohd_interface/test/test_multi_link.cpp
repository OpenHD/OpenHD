#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "multi_link.h"

class FakeLink final : public OHDLink {
 public:
  void transmit_telemetry_data(TelemetryTxPacket packet) override {
    last_telemetry = std::move(packet.data);
    last_n_injections = packet.n_injections;
    last_packet_type = packet.packet_type;
    telemetry_tx.fetch_add(1);
  }

  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override {
    last_stream_index = stream_index;
    last_frame = fragmented_video_frame;
    video_tx.fetch_add(1);
  }

  void transmit_audio_data(const openhd::AudioPacket&) override {
    audio_tx.fetch_add(1);
  }

  std::atomic<int> telemetry_tx{0};
  std::atomic<int> video_tx{0};
  std::atomic<int> audio_tx{0};
  int last_stream_index = -1;
  std::shared_ptr<std::vector<uint8_t>> last_telemetry;
  int last_n_injections = 0;
  TelemetryPacketType last_packet_type = TelemetryPacketType::Telemetry;
  openhd::FragmentedVideoFrame last_frame;
};

class BlockingLink final : public OHDLink {
 public:
  void transmit_telemetry_data(TelemetryTxPacket) override {}
  void transmit_video_data(int, const openhd::FragmentedVideoFrame&) override {
    std::unique_lock<std::mutex> lock(mutex);
    entered = true;
    changed.notify_all();
    changed.wait(lock, [this]() { return released; });
  }
  void transmit_audio_data(const openhd::AudioPacket&) override {}

  void wait_until_entered() {
    std::unique_lock<std::mutex> lock(mutex);
    assert(changed.wait_for(lock, std::chrono::seconds(2),
                            [this]() { return entered; }));
  }
  void release() {
    std::lock_guard<std::mutex> lock(mutex);
    released = true;
    changed.notify_all();
  }

 private:
  std::mutex mutex;
  std::condition_variable changed;
  bool entered = false;
  bool released = false;
};

template <class Predicate>
void wait_for(Predicate predicate) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!predicate() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  assert(predicate());
}

int main() {
  auto first = std::make_shared<FakeLink>();
  auto second = std::make_shared<FakeLink>();
  MultiLink multi;
  multi.add_link("first", first);
  multi.add_link("second", second);
  assert(multi.link_count() == 2);

  auto rtp =
      std::make_shared<std::vector<uint8_t>>(std::initializer_list<uint8_t>{
          0x80, 96, 0x12, 0x34, 0, 0, 0, 1, 0xaa, 0xbb, 0xcc, 0xdd, 1, 2, 3});
  openhd::FragmentedVideoFrame frame;
  frame.rtp_fragments.push_back(rtp);
  multi.transmit_video_data(0, frame);
  wait_for([&]() { return first->video_tx == 1 && second->video_tx == 1; });
  assert(first->video_tx == 1);
  assert(second->video_tx == 1);
  assert(first->last_frame.rtp_fragments.front() == rtp);
  assert(second->last_frame.rtp_fragments.front() == rtp);

  auto telemetry = std::make_shared<std::vector<uint8_t>>(
      std::initializer_list<uint8_t>{1, 2, 3, 4});
  multi.transmit_telemetry_data(
      {telemetry, 4, OHDLink::TelemetryPacketType::RC});
  wait_for(
      [&]() { return first->telemetry_tx == 1 && second->telemetry_tx == 1; });
  assert(first->telemetry_tx == 1);
  assert(second->telemetry_tx == 1);
  assert(first->last_n_injections == 4);
  assert(second->last_n_injections == 4);
  assert(first->last_packet_type == OHDLink::TelemetryPacketType::RC);
  assert(second->last_packet_type == OHDLink::TelemetryPacketType::RC);

  int received_video = 0;
  multi.register_on_receive_video_data_cb(
      [&received_video](int, const uint8_t*, int) { ++received_video; });
  first->on_receive_video_data(0, rtp->data(), rtp->size());
  second->on_receive_video_data(0, rtp->data(), rtp->size());
  assert(received_video == 1);

  int received_telemetry = 0;
  multi.register_on_receive_telemetry_data_cb(
      [&received_telemetry](std::shared_ptr<std::vector<uint8_t>>) {
        ++received_telemetry;
      });
  first->on_receive_telemetry_data(telemetry);
  second->on_receive_telemetry_data(telemetry);
  assert(received_telemetry == 1);

  multi.remove_link(first);
  multi.transmit_video_data(1, frame);
  wait_for([&]() { return second->video_tx == 2; });
  assert(first->video_tx == 1);
  assert(second->video_tx == 2);

  auto blocked = std::make_shared<BlockingLink>();
  auto independent = std::make_shared<FakeLink>();
  MultiLink isolated;
  isolated.add_link("blocked", blocked);
  isolated.add_link("independent", independent);
  isolated.transmit_video_data(0, frame);
  blocked->wait_until_entered();
  wait_for([&]() { return independent->video_tx == 1; });
  blocked->release();
  return 0;
}
