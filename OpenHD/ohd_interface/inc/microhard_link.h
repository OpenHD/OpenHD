//
// Created by consti10 on 05.04.24.
//

#ifndef OPENHD_MICROHARD_LINK_H
#define OPENHD_MICROHARD_LINK_H

#include "openhd_link.hpp"
#include "openhd_settings_imp.h"
#include "openhd_udp.h"

/**
 * Experimental link implementation for microhard modules
 * (Or anything else really using IP addresses)
 */
class MicrohardLink : public OHDLink {
 public:
  explicit MicrohardLink(OHDProfile profile);
  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;
  /**
   * @return all mavlink settings, values might change depending on air/ground
   * and/or the used hardware
   */
  std::vector<openhd::Setting> get_all_settings();

 private:
  const OHDProfile m_profile;
  std::unique_ptr<openhd::UDPForwarder> m_video_tx;
  std::unique_ptr<openhd::UDPReceiver> m_video_rx;
  //
  std::unique_ptr<openhd::UDPReceiver> m_telemetry_tx_rx;
};

#endif  // OPENHD_MICROHARD_LINK_H
