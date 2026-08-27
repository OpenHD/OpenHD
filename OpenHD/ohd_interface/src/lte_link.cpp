#include "lte_link.h"

#include <utility>

#include "openhd_spdlog.h"

LteLink::LteLink(LteLinkConfig config) : m_config(std::move(config)) {
  m_video_tx = std::make_unique<openhd::UDPForwarder>(
      m_config.fleetcontrol_address, m_config.video_port);
  m_video2_tx = std::make_unique<openhd::UDPForwarder>(
      m_config.fleetcontrol_address, m_config.video2_port);
  m_telemetry = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", 0,
      [this](const uint8_t* data, std::size_t size) {
        on_receive_telemetry_data(
            std::make_shared<std::vector<uint8_t>>(data, data + size));
      });
  m_telemetry->runInBackground();
  openhd::log::get_default()->info(
      "LTE link {} -> {} (video {}/{}, telemetry {})", m_config.device_id,
      m_config.fleetcontrol_address, m_config.video_port,
      m_config.video2_port, m_config.telemetry_port);
}

LteLink::~LteLink() {
  if (m_telemetry) m_telemetry->stopBackground();
}

void LteLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  if (!packet.data || packet.data->empty()) return;
  // Reuse one ephemeral listening socket so FleetControl can reply to the
  // observed WireGuard source endpoint without colliding with local MAVLink
  // services. n_injections remains WFB-specific.
  m_telemetry->forwardPacketViaUDP(
      m_config.fleetcontrol_address, m_config.telemetry_port,
      packet.data->data(), packet.data->size());
}

void LteLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  auto* destination = stream_index == 0 ? m_video_tx.get() : m_video2_tx.get();
  if (!destination) return;
  for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
    if (fragment && !fragment->empty()) {
      destination->forwardPacketViaUDP(fragment->data(), fragment->size());
    }
  }
}

void LteLink::transmit_audio_data(const openhd::AudioPacket&) {}
