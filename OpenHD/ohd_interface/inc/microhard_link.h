/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_MICROHARD_LINK_H
#define OPENHD_MICROHARD_LINK_H

#include <atomic>
#include <cstdint>
#include <thread>

#include "non_wb_video_bitrate_meter.h"
#include "openhd_link.hpp"
#include "openhd_settings_imp.h"
#include "openhd_udp.h"

/**
 * Link implementation for microhard modules
 */
class MicrohardLink : public OHDLink {
 public:
  explicit MicrohardLink(OHDProfile profile);
  ~MicrohardLink();
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
  static void monitor_gateway_signal_strength(const std::string& gateway_ip);

 private:
  void start_stats_thread();
  void stop_stats_thread();
  void stats_loop();
  void update_link_stats();

 private:
  const OHDProfile m_profile;
  std::unique_ptr<openhd::UDPForwarder> m_video_tx;
  std::unique_ptr<openhd::UDPReceiver> m_video_rx;
  //
  std::unique_ptr<openhd::UDPReceiver> m_telemetry_tx_rx;
  std::atomic<bool> m_stats_running{false};
  std::thread m_stats_thread;
  std::atomic<uint64_t> m_tx_total_bytes{0};
  std::atomic<uint64_t> m_tx_total_packets{0};
  std::atomic<uint64_t> m_rx_total_bytes{0};
  std::atomic<uint64_t> m_rx_total_packets{0};
  std::atomic<uint64_t> m_tx_tele_bytes{0};
  std::atomic<uint64_t> m_tx_tele_packets{0};
  std::atomic<uint64_t> m_rx_tele_bytes{0};
  std::atomic<uint64_t> m_rx_tele_packets{0};
  std::atomic<int64_t> m_last_rx_packet_ts_ms{0};
  int64_t m_last_stats_ts_ms = 0;
  uint64_t m_last_stats_tx_bytes = 0;
  uint64_t m_last_stats_tx_packets = 0;
  uint64_t m_last_stats_rx_bytes = 0;
  uint64_t m_last_stats_rx_packets = 0;
  uint64_t m_last_stats_tx_tele_bytes = 0;
  uint64_t m_last_stats_tx_tele_packets = 0;
  uint64_t m_last_stats_rx_tele_bytes = 0;
  uint64_t m_last_stats_rx_tele_packets = 0;
  openhd::non_wb::VideoBitrateMeter m_video_bitrate_meter;
};

#endif  // OPENHD_MICROHARD_LINK_H
