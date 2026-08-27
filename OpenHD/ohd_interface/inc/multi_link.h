#ifndef OPENHD_MULTI_LINK_H
#define OPENHD_MULTI_LINK_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "openhd_link.hpp"

// Stable fan-out/fan-in facade used by video and telemetry. Transport
// implementations remain independent OHDLink instances and can be added or
// removed without reconnecting the media pipeline to another link object.
class MultiLink final : public OHDLink {
 public:
  MultiLink() = default;
  ~MultiLink() override;
  MultiLink(const MultiLink&) = delete;
  MultiLink& operator=(const MultiLink&) = delete;

  void add_link(std::string name, std::shared_ptr<OHDLink> link);
  void remove_link(const std::shared_ptr<OHDLink>& link);
  void clear_links();
  [[nodiscard]] std::size_t link_count() const;
  [[nodiscard]] std::vector<std::string> link_names() const;

  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;

 private:
  struct Endpoint {
    std::string name;
    std::shared_ptr<OHDLink> link;
    std::mutex queue_mutex;
    std::condition_variable queue_changed;
    bool stopping = false;
    std::deque<TelemetryTxPacket> telemetry_queue;
    struct PendingVideo {
      int stream_index;
      openhd::FragmentedVideoFrame frame;
    };
    std::deque<PendingVideo> video_queue;
    std::deque<openhd::AudioPacket> audio_queue;
    std::thread worker;
  };

  struct RecentPacket {
    std::chrono::steady_clock::time_point received_at;
  };

  struct CallbackGate {
    explicit CallbackGate(MultiLink* owner1) : owner(owner1) {}
    std::mutex mutex;
    MultiLink* owner = nullptr;
  };

  using EndpointPtr = std::shared_ptr<Endpoint>;
  std::vector<EndpointPtr> endpoints_snapshot() const;
  static void dispatch_loop(const EndpointPtr& endpoint);
  static void stop_endpoint(const EndpointPtr& endpoint);
  static void enqueue_telemetry(const EndpointPtr& endpoint,
                                TelemetryTxPacket packet);
  static void enqueue_video(
      const EndpointPtr& endpoint, int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame);
  static void enqueue_audio(const EndpointPtr& endpoint,
                            const openhd::AudioPacket& audio_packet);
  bool is_duplicate_video(int stream_index, const uint8_t* data, int data_len);
  bool is_duplicate_data(const uint8_t* data, int data_len,
                         std::unordered_map<uint64_t, RecentPacket>& recent,
                         std::chrono::milliseconds lifetime);
  void prune_recent_locked(std::unordered_map<uint64_t, RecentPacket>& recent,
                           std::chrono::steady_clock::time_point now,
                           std::chrono::milliseconds lifetime);

  mutable std::mutex m_endpoints_mutex;
  std::vector<EndpointPtr> m_endpoints;
  std::mutex m_dedup_mutex;
  std::unordered_map<uint64_t, RecentPacket> m_recent_video;
  std::unordered_map<uint64_t, RecentPacket> m_recent_telemetry;
  std::unordered_map<uint64_t, RecentPacket> m_recent_audio;
  std::shared_ptr<CallbackGate> m_callback_gate =
      std::make_shared<CallbackGate>(this);
};

#endif  // OPENHD_MULTI_LINK_H
