#include "multi_link.h"

#include <algorithm>
#include <utility>

#include "openhd_secondary_telemetry.hpp"

namespace {
constexpr auto kVideoDuplicateLifetime = std::chrono::seconds(2);
constexpr auto kDataDuplicateLifetime = std::chrono::milliseconds(250);
constexpr std::size_t kRecentPacketSoftLimit = 8192;
constexpr std::size_t kTelemetryQueueLimit = 128;
constexpr std::size_t kVideoQueueLimitPerStream = 2;
constexpr std::size_t kAudioQueueLimit = 32;

uint64_t fnv1a(const uint8_t* data, int data_len) {
  uint64_t hash = 1469598103934665603ULL;
  for (int i = 0; i < data_len; ++i) {
    hash ^= data[i];
    hash *= 1099511628211ULL;
  }
  hash ^= static_cast<uint64_t>(data_len);
  hash *= 1099511628211ULL;
  return hash;
}

uint64_t video_packet_key(int stream_index, const uint8_t* data, int data_len) {
  // RTP V2: sequence number is bytes 2..3 and SSRC is bytes 8..11. All OpenHD
  // transports carry the same RTP packet, so this key identifies duplicates
  // without hashing the video payload.
  if (data_len >= 12 && (data[0] >> 6U) == 2U) {
    const uint64_t sequence = (static_cast<uint64_t>(data[2]) << 8U) | data[3];
    const uint64_t ssrc = (static_cast<uint64_t>(data[8]) << 24U) |
                          (static_cast<uint64_t>(data[9]) << 16U) |
                          (static_cast<uint64_t>(data[10]) << 8U) | data[11];
    return (static_cast<uint64_t>(stream_index & 0xff) << 56U) | (ssrc << 16U) |
           sequence;
  }
  return fnv1a(data, data_len) ^
         (static_cast<uint64_t>(stream_index & 0xff) << 56U);
}
}  // namespace

MultiLink::~MultiLink() {
  clear_links();
  std::lock_guard<std::mutex> lock(m_callback_gate->mutex);
  m_callback_gate->owner = nullptr;
}

void MultiLink::clear_links() {
  std::vector<EndpointPtr> endpoints;
  {
    std::lock_guard<std::mutex> lock(m_endpoints_mutex);
    endpoints.swap(m_endpoints);
  }
  for (const auto& endpoint : endpoints) {
    if (endpoint->name != "WIFIBROADCAST") {
      openhd::SecondaryTelemetryStatus::instance().set_configured(
          endpoint->name, false);
    }
    endpoint->link->register_on_receive_video_data_cb(nullptr);
    endpoint->link->register_on_receive_telemetry_data_cb(nullptr);
    endpoint->link->m_audio_data_rx_cb = nullptr;
    stop_endpoint(endpoint);
  }
}

void MultiLink::add_link(std::string name, std::shared_ptr<OHDLink> link) {
  if (!link) return;
  auto endpoint = std::make_shared<Endpoint>();
  endpoint->name = std::move(name);
  endpoint->link = link;
  {
    std::lock_guard<std::mutex> lock(m_endpoints_mutex);
    const auto existing = std::find_if(m_endpoints.begin(), m_endpoints.end(),
                                       [&link](const EndpointPtr& candidate) {
                                         return candidate->link == link;
                                       });
    if (existing != m_endpoints.end()) return;
    m_endpoints.push_back(endpoint);
  }
  if (endpoint->name != "WIFIBROADCAST") {
    openhd::SecondaryTelemetryStatus::instance().set_configured(endpoint->name,
                                                                true);
  }
  endpoint->worker = std::thread([endpoint]() { dispatch_loop(endpoint); });
  auto gate = m_callback_gate;
  link->register_on_receive_video_data_cb(
      [gate](int stream_index, const uint8_t* data, int data_len) {
        std::lock_guard<std::mutex> lock(gate->mutex);
        if (gate->owner &&
            !gate->owner->is_duplicate_video(stream_index, data, data_len)) {
          gate->owner->on_receive_video_data(stream_index, data, data_len);
        }
      });
  link->register_on_receive_telemetry_data_cb(
      [gate, endpoint](std::shared_ptr<std::vector<uint8_t>> data) {
        if (data && !data->empty() && endpoint->name != "WIFIBROADCAST") {
          openhd::SecondaryTelemetryStatus::instance().note_received(
              endpoint->name);
        }
        std::lock_guard<std::mutex> lock(gate->mutex);
        if (gate->owner && data &&
            !gate->owner->is_duplicate_data(
                data->data(), static_cast<int>(data->size()),
                gate->owner->m_recent_telemetry, kDataDuplicateLifetime)) {
          gate->owner->on_receive_telemetry_data(std::move(data));
        }
      });
  link->m_audio_data_rx_cb = [gate](const uint8_t* data, int data_len) {
    std::lock_guard<std::mutex> lock(gate->mutex);
    if (gate->owner && !gate->owner->is_duplicate_data(
                           data, data_len, gate->owner->m_recent_audio,
                           kDataDuplicateLifetime)) {
      gate->owner->on_receive_audio_data(data, data_len);
    }
  };
}

void MultiLink::remove_link(const std::shared_ptr<OHDLink>& link) {
  if (!link) return;
  EndpointPtr removed;
  {
    std::lock_guard<std::mutex> lock(m_endpoints_mutex);
    const auto existing = std::find_if(m_endpoints.begin(), m_endpoints.end(),
                                       [&link](const EndpointPtr& endpoint) {
                                         return endpoint->link == link;
                                       });
    if (existing == m_endpoints.end()) return;
    removed = *existing;
    m_endpoints.erase(existing);
  }
  removed->link->register_on_receive_video_data_cb(nullptr);
  removed->link->register_on_receive_telemetry_data_cb(nullptr);
  removed->link->m_audio_data_rx_cb = nullptr;
  if (removed->name != "WIFIBROADCAST") {
    openhd::SecondaryTelemetryStatus::instance().set_configured(removed->name,
                                                                false);
  }
  stop_endpoint(removed);
}

std::size_t MultiLink::link_count() const {
  std::lock_guard<std::mutex> lock(m_endpoints_mutex);
  return m_endpoints.size();
}

std::vector<std::string> MultiLink::link_names() const {
  std::vector<std::string> result;
  std::lock_guard<std::mutex> lock(m_endpoints_mutex);
  result.reserve(m_endpoints.size());
  for (const auto& endpoint : m_endpoints) result.push_back(endpoint->name);
  return result;
}

std::vector<MultiLink::EndpointPtr> MultiLink::endpoints_snapshot() const {
  std::lock_guard<std::mutex> lock(m_endpoints_mutex);
  return m_endpoints;
}

void MultiLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  for (const auto& endpoint : endpoints_snapshot()) {
    enqueue_telemetry(endpoint, packet);
  }
}

void MultiLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  for (const auto& endpoint : endpoints_snapshot()) {
    enqueue_video(endpoint, stream_index, fragmented_video_frame);
  }
}

void MultiLink::transmit_audio_data(const openhd::AudioPacket& audio_packet) {
  for (const auto& endpoint : endpoints_snapshot()) {
    enqueue_audio(endpoint, audio_packet);
  }
}

void MultiLink::dispatch_loop(const EndpointPtr& endpoint) {
  while (true) {
    TelemetryTxPacket telemetry;
    Endpoint::PendingVideo video;
    openhd::AudioPacket audio;
    bool has_telemetry = false;
    bool has_video = false;
    bool has_audio = false;
    {
      std::unique_lock<std::mutex> lock(endpoint->queue_mutex);
      endpoint->queue_changed.wait(lock, [&endpoint]() {
        return endpoint->stopping || !endpoint->telemetry_queue.empty() ||
               !endpoint->video_queue.empty() || !endpoint->audio_queue.empty();
      });
      if (endpoint->stopping) return;
      if (!endpoint->telemetry_queue.empty()) {
        auto next = std::find_if(
            endpoint->telemetry_queue.begin(), endpoint->telemetry_queue.end(),
            [](const TelemetryTxPacket& queued) {
              return queued.packet_type == TelemetryPacketType::RC;
            });
        if (next == endpoint->telemetry_queue.end()) {
          next = endpoint->telemetry_queue.begin();
        }
        telemetry = std::move(*next);
        endpoint->telemetry_queue.erase(next);
        has_telemetry = true;
      }
      if (!endpoint->video_queue.empty()) {
        video = std::move(endpoint->video_queue.front());
        endpoint->video_queue.pop_front();
        has_video = true;
      }
      if (!endpoint->audio_queue.empty()) {
        audio = std::move(endpoint->audio_queue.front());
        endpoint->audio_queue.pop_front();
        has_audio = true;
      }
    }
    // A transport exception must only take out that dispatch iteration. The
    // producer and all other transports remain independent.
    try {
      if (has_telemetry) {
        endpoint->link->transmit_telemetry_data(std::move(telemetry));
      }
      if (has_video) {
        endpoint->link->transmit_video_data(video.stream_index, video.frame);
      }
      if (has_audio) endpoint->link->transmit_audio_data(audio);
    } catch (...) {
    }
  }
}

void MultiLink::stop_endpoint(const EndpointPtr& endpoint) {
  {
    std::lock_guard<std::mutex> lock(endpoint->queue_mutex);
    endpoint->stopping = true;
    endpoint->telemetry_queue.clear();
    endpoint->video_queue.clear();
    endpoint->audio_queue.clear();
  }
  endpoint->queue_changed.notify_one();
  if (endpoint->worker.joinable()) {
    if (endpoint->worker.get_id() == std::this_thread::get_id()) {
      // A transport may quarantine itself from inside transmit(). The worker
      // owns endpoint until dispatch_loop returns, so detaching its final
      // iteration is safe and avoids joining the current thread.
      endpoint->worker.detach();
    } else {
      endpoint->worker.join();
    }
  }
}

void MultiLink::enqueue_telemetry(const EndpointPtr& endpoint,
                                  TelemetryTxPacket packet) {
  {
    std::lock_guard<std::mutex> lock(endpoint->queue_mutex);
    if (endpoint->stopping) return;
    if (endpoint->telemetry_queue.size() >= kTelemetryQueueLimit) {
      // Prefer retaining RC control packets if the transport is congested.
      auto drop = endpoint->telemetry_queue.begin();
      if (packet.packet_type == TelemetryPacketType::RC) {
        drop = std::find_if(
            endpoint->telemetry_queue.begin(), endpoint->telemetry_queue.end(),
            [](const TelemetryTxPacket& queued) {
              return queued.packet_type != TelemetryPacketType::RC;
            });
      }
      if (drop == endpoint->telemetry_queue.end()) {
        drop = endpoint->telemetry_queue.begin();
      }
      endpoint->telemetry_queue.erase(drop);
    }
    endpoint->telemetry_queue.push_back(std::move(packet));
  }
  endpoint->queue_changed.notify_one();
}

void MultiLink::enqueue_video(
    const EndpointPtr& endpoint, int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  {
    std::lock_guard<std::mutex> lock(endpoint->queue_mutex);
    if (endpoint->stopping) return;
    if (fragmented_video_frame.is_idr_frame) {
      endpoint->video_queue.erase(
          std::remove_if(endpoint->video_queue.begin(),
                         endpoint->video_queue.end(),
                         [stream_index](const Endpoint::PendingVideo& queued) {
                           return queued.stream_index == stream_index;
                         }),
          endpoint->video_queue.end());
    }
    std::size_t stream_queue_size = 0;
    for (const auto& queued : endpoint->video_queue) {
      if (queued.stream_index == stream_index) ++stream_queue_size;
    }
    if (stream_queue_size >= kVideoQueueLimitPerStream) {
      const auto oldest = std::find_if(
          endpoint->video_queue.begin(), endpoint->video_queue.end(),
          [stream_index](const Endpoint::PendingVideo& queued) {
            return queued.stream_index == stream_index;
          });
      if (oldest != endpoint->video_queue.end()) {
        endpoint->video_queue.erase(oldest);
      }
    }
    endpoint->video_queue.push_back(
        Endpoint::PendingVideo{stream_index, fragmented_video_frame});
  }
  endpoint->queue_changed.notify_one();
}

void MultiLink::enqueue_audio(const EndpointPtr& endpoint,
                              const openhd::AudioPacket& audio_packet) {
  {
    std::lock_guard<std::mutex> lock(endpoint->queue_mutex);
    if (endpoint->stopping) return;
    if (endpoint->audio_queue.size() >= kAudioQueueLimit) {
      endpoint->audio_queue.pop_front();
    }
    endpoint->audio_queue.push_back(audio_packet);
  }
  endpoint->queue_changed.notify_one();
}

bool MultiLink::is_duplicate_video(int stream_index, const uint8_t* data,
                                   int data_len) {
  if (!data || data_len <= 0) return true;
  const auto key = video_packet_key(stream_index, data, data_len);
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard<std::mutex> lock(m_dedup_mutex);
  const auto found = m_recent_video.find(key);
  if (found != m_recent_video.end() &&
      now - found->second.received_at <= kVideoDuplicateLifetime) {
    return true;
  }
  m_recent_video[key] = RecentPacket{now};
  if (m_recent_video.size() > kRecentPacketSoftLimit) {
    prune_recent_locked(m_recent_video, now, kVideoDuplicateLifetime);
  }
  return false;
}

bool MultiLink::is_duplicate_data(
    const uint8_t* data, int data_len,
    std::unordered_map<uint64_t, RecentPacket>& recent,
    std::chrono::milliseconds lifetime) {
  if (!data || data_len <= 0) return true;
  const auto key = fnv1a(data, data_len);
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard<std::mutex> lock(m_dedup_mutex);
  const auto found = recent.find(key);
  if (found != recent.end() && now - found->second.received_at <= lifetime) {
    return true;
  }
  recent[key] = RecentPacket{now};
  if (recent.size() > kRecentPacketSoftLimit) {
    prune_recent_locked(recent, now, lifetime);
  }
  return false;
}

void MultiLink::prune_recent_locked(
    std::unordered_map<uint64_t, RecentPacket>& recent,
    std::chrono::steady_clock::time_point now,
    std::chrono::milliseconds lifetime) {
  for (auto it = recent.begin(); it != recent.end();) {
    if (now - it->second.received_at > lifetime) {
      it = recent.erase(it);
    } else {
      ++it;
    }
  }
}
