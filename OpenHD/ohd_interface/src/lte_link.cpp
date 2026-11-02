#include "lte_link.h"

#include <utility>

#include "openhd_spdlog.h"
#include "openhd_action_handler.h"
#include "openhd_link_usage.hpp"

LteLink::LteLink(LteLinkConfig config) : m_config(std::move(config)) {
  m_video[0] = std::make_unique<openhd::FleetVideoLease>(m_config.fleetcontrol_address, m_config.video_port, true);
  m_video[1] = std::make_unique<openhd::FleetVideoLease>(m_config.fleetcontrol_address, m_config.video2_port, true);
  m_stats = std::thread([this] {
    while (!m_stopping) {
      {
        std::lock_guard<std::mutex> lock(m_video_mutex);
        m_video[0]->allowed();
        const auto metrics = m_video[0]->metrics();
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        const int budget = m_rate.update(openhd::bandwidth::kMaximumEncoderPoolKbits, 0, metrics.rtt_us, metrics.probe_loss_permille, m_send_errors, now);
        if (m_config.is_air) {
          openhd::link_usage::Registry::instance().set_capacity("LTE", budget * 1000);
          openhd::LinkActionHandler::instance().action_request_bitrate_change_handle(
              {budget, true, openhd::bandwidth::Transport::Lte});
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });
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
  m_stopping = true;
  if (m_stats.joinable()) m_stats.join();
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
  openhd::link_usage::Registry::instance().record("LTE", packet.packet_type == TelemetryPacketType::RC ? openhd::link_usage::Category::Control : openhd::link_usage::Category::Telemetry, packet.data->size());
}

void LteLink::transmit_video_data(
    int index, const openhd::FragmentedVideoFrame& frame) {
  if (index < 0 || index > 1) return;
  std::lock_guard<std::mutex> lock(m_video_mutex);
  if (!m_video[index]->allowed()) return;
  for (const auto& fragment : frame.rtp_fragments) {
    if (!fragment || fragment->empty()) continue;
    if (!m_video[index]->send_rtp(fragment->data(), fragment->size())) ++m_send_errors;
    else openhd::link_usage::Registry::instance().record("LTE", index == 0 ? openhd::link_usage::Category::Camera1 : openhd::link_usage::Category::Camera2, fragment->size());
  }
}

void LteLink::transmit_audio_data(const openhd::AudioPacket&) {}
