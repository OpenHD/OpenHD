#ifndef OPENHD_LTE_LINK_H
#define OPENHD_LTE_LINK_H

#include <memory>
#include <string>

#include "openhd_link.hpp"
#include "openhd_udp.h"
#include "openhd_fleet_video_lease.hpp"
#include "openhd_transport_bitrate.hpp"
#include <array>
#include <mutex>
#include <thread>
#include <atomic>

struct LteLinkConfig {
  std::string device_id;
  std::string fleetcontrol_address;
  int video_port = 0;
  int video2_port = 0;
  int telemetry_port = 0;
  bool is_air = true;
};

// An OHDLink carried by WireGuard. WireGuard owns encryption and peer
// identity; this class only moves the original RTP and MAVLink datagrams.
class LteLink final : public OHDLink {
 public:
  explicit LteLink(LteLinkConfig config);
  ~LteLink() override;

  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;
  bool usage_supports_audio() override { return false; }
  bool records_own_usage() const override { return true; }
  bool usage_supports_video(int) override { return true; }

 private:
  LteLinkConfig m_config;
  std::unique_ptr<openhd::UDPReceiver> m_telemetry;
  std::array<std::unique_ptr<openhd::FleetVideoLease>, 2> m_video;
  std::mutex m_video_mutex;
  std::atomic_bool m_stopping{false};
  std::thread m_stats;
  openhd::bandwidth::RateController m_rate;
  uint32_t m_send_errors = 0;
};

#endif  // OPENHD_LTE_LINK_H
