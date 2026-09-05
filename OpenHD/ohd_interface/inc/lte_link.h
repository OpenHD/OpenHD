#ifndef OPENHD_LTE_LINK_H
#define OPENHD_LTE_LINK_H

#include <memory>
#include <string>

#include "openhd_link.hpp"
#include "openhd_udp.h"

struct LteLinkConfig {
  std::string device_id;
  std::string fleetcontrol_address;
  int video_port = 0;
  int video2_port = 0;
  int telemetry_port = 0;
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

 private:
  LteLinkConfig m_config;
  std::unique_ptr<openhd::UDPReceiver> m_telemetry;
};

#endif  // OPENHD_LTE_LINK_H
