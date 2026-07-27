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

#ifndef OPENHD_ETHERNET_LINK_H
#define OPENHD_ETHERNET_LINK_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "non_wb_video_bitrate_meter.h"
#include "openhd_config.h"
#include "openhd_link.hpp"
#include "openhd_udp.h"
#include "openhd_util.h"

class EthernetLink : public OHDLink {
 public:
  EthernetLink(const openhd::Config& config, OHDProfile profile);
  EthernetLink(OHDProfile profile);
  EthernetLink(OHDProfile profile, bool auto_discovery);
  ~EthernetLink();

  // OHDLink implementations
  void transmit_telemetry_data(TelemetryTxPacket packet) override;
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;

 private:
  OHDProfile m_profile;
  openhd::Config m_config;
  // Configuration variables (defaults if not overridden)
  std::string GROUND_UNIT_IP = "192.168.2.1";
  std::string AIR_UNIT_IP = "192.168.2.18";
  int VIDEO_PORT = 5910;
  int TELEMETRY_PORT = 5920;

  std::shared_ptr<openhd::UDPForwarder> m_video_tx;  // Video transmitter
  std::unique_ptr<openhd::UDPReceiver> m_video_rx;   // Video receiver
  std::shared_ptr<openhd::UDPForwarder>
      m_telemetry_tx;                                   // Telemetry transmitter
  std::unique_ptr<openhd::UDPReceiver> m_telemetry_rx;  // Telemetry receiver

  void initialize(bool auto_discovery);
  void initialize_air_unit();
  void initialize_ground_unit();
  void configure_peer(const std::string& peer_ip, int video_port,
                      int telemetry_port);
  void start_discovery();
  void stop_discovery();
  void discovery_loop();
  void configure_static_address_if_dhcp_is_unavailable();
  void air_discovery_loop(int socket_fd);
  void ground_discovery_loop(int socket_fd);
  void start_stats_thread();
  void stop_stats_thread();
  void stats_loop();
  void update_link_stats();

  void handle_video_data(int stream_index, const uint8_t* data, int data_len);
  void handle_telemetry_data(const uint8_t* data, int data_len);

  bool m_auto_discovery = false;
  std::atomic<bool> m_discovery_running{false};
  std::thread m_discovery_thread;
  std::mutex m_forwarders_mutex;
  std::string m_peer_ip;
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

#endif  // OPENHD_ETHERNET_LINK_H
