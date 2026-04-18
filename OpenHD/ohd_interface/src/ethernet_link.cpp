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

#include "ethernet_link.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <limits>

#include "config_paths.h"
#include "openhd_action_handler.h"
#include "openhd_config.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_util_time.h"

namespace {
int16_t clamp_int16(int value) {
  if (value > std::numeric_limits<int16_t>::max()) {
    return std::numeric_limits<int16_t>::max();
  }
  if (value < std::numeric_limits<int16_t>::min()) {
    return std::numeric_limits<int16_t>::min();
  }
  return static_cast<int16_t>(value);
}

int32_t clamp_int32(int64_t value) {
  if (value > std::numeric_limits<int32_t>::max()) {
    return std::numeric_limits<int32_t>::max();
  }
  if (value < std::numeric_limits<int32_t>::min()) {
    return std::numeric_limits<int32_t>::min();
  }
  return static_cast<int32_t>(value);
}
}  // namespace

static std::string ETHERNET_FILE_PATH =
    std::string(getConfigBasePath()) + "ethernet.txt";

EthernetLink::EthernetLink(const openhd::Config& config, OHDProfile profile)
    : m_config(config), m_profile(profile) {
  std::cout << "ethernet starting " << std::endl;

  if (OHDFilesystemUtil::exists(ETHERNET_FILE_PATH)) {
    const auto config = openhd::load_config();
    std::cout << "ethernet config load " << std::endl;

    try {
      // Assign to class member variables instead of local static variables
      GROUND_UNIT_IP = config.GROUND_UNIT_IP;
      AIR_UNIT_IP = config.AIR_UNIT_IP;
      VIDEO_PORT = config.VIDEO_PORT;
      TELEMETRY_PORT = config.TELEMETRY_PORT;

      // Debugging the values after assignment
      std::cout << "Assigned ethernet parameters:" << std::endl;
      std::cout << "  GROUND_UNIT_IP: " << GROUND_UNIT_IP << std::endl;
      std::cout << "  AIR_UNIT_IP: " << AIR_UNIT_IP << std::endl;
      std::cout << "  VIDEO_PORT: " << VIDEO_PORT << std::endl;
      std::cout << "  TELEMETRY_PORT: " << TELEMETRY_PORT << std::endl;
    } catch (const std::exception& ex) {
      std::cerr << "Failed to read ethernet parameters: " << ex.what()
                << std::endl;
      throw;
    }
  } else {
    std::cerr << "Ethernet parameters not found. Using default configuration."
              << std::endl;
  }

  // Initialize either air or ground unit based on the profile
  if (m_profile.is_air) {
    initialize_air_unit();
  } else {
    initialize_ground_unit();
  }
  start_stats_thread();
}

EthernetLink::EthernetLink(OHDProfile profile)
    : EthernetLink(openhd::load_config(), profile) {}

EthernetLink::~EthernetLink() {
  stop_stats_thread();
  // Stop background receivers
  if (m_video_rx) m_video_rx->stopBackground();
  if (m_telemetry_rx) m_telemetry_rx->stopBackground();
}

void EthernetLink::initialize_air_unit() {
  // Initialize video transmitter for sending video to the ground unit
  m_video_tx =
      std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, VIDEO_PORT);

  // Initialize telemetry transmitter and receiver for bidirectional telemetry
  m_telemetry_tx =
      std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, TELEMETRY_PORT);
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start telemetry receiver in the background
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::initialize_ground_unit() {
  // Initialize video receiver for receiving video from the air unit
  m_video_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", VIDEO_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_video_data(0, data, len);  // Process incoming video
      });

  // Initialize telemetry transmitter and receiver for bidirectional telemetry
  m_telemetry_tx =
      std::make_unique<openhd::UDPForwarder>(AIR_UNIT_IP, TELEMETRY_PORT);
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start video and telemetry receivers in the background
  if (m_video_rx) m_video_rx->runInBackground();
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  // Send telemetry data to the destination
  if (m_telemetry_tx) {
    m_telemetry_tx->forwardPacketViaUDP(packet.data->data(),
                                        packet.data->size());
    m_tx_total_bytes.fetch_add(static_cast<uint64_t>(packet.data->size()),
                               std::memory_order_relaxed);
    m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
    m_tx_tele_bytes.fetch_add(static_cast<uint64_t>(packet.data->size()),
                              std::memory_order_relaxed);
    m_tx_tele_packets.fetch_add(1, std::memory_order_relaxed);
  }
}

void EthernetLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  // Send video data fragments to the destination
  if (m_video_tx) {
    for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
      m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
      m_video_bitrate_meter.on_tx_fragment(
          stream_index, static_cast<uint64_t>(fragment->size()));
      m_tx_total_bytes.fetch_add(static_cast<uint64_t>(fragment->size()),
                                 std::memory_order_relaxed);
      m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
    }
  }
}

void EthernetLink::transmit_audio_data(
    const openhd::AudioPacket& audio_packet) {
  // Currently not implemented for EthernetLink
}

void EthernetLink::handle_video_data(int stream_index, const uint8_t* data,
                                     int data_len) {
  m_rx_total_bytes.fetch_add(static_cast<uint64_t>(data_len),
                             std::memory_order_relaxed);
  m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
  m_last_rx_packet_ts_ms.store(openhd::util::steady_clock_time_epoch_ms(),
                               std::memory_order_relaxed);
  // Forward incoming video data to the upper layer
  on_receive_video_data(stream_index, data, data_len);
}

void EthernetLink::handle_telemetry_data(const uint8_t* data, int data_len) {
  m_rx_total_bytes.fetch_add(static_cast<uint64_t>(data_len),
                             std::memory_order_relaxed);
  m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
  m_rx_tele_bytes.fetch_add(static_cast<uint64_t>(data_len),
                            std::memory_order_relaxed);
  m_rx_tele_packets.fetch_add(1, std::memory_order_relaxed);
  m_last_rx_packet_ts_ms.store(openhd::util::steady_clock_time_epoch_ms(),
                               std::memory_order_relaxed);
  // Forward incoming telemetry data to the upper layer
  auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
  on_receive_telemetry_data(shared);
}

void EthernetLink::start_stats_thread() {
  if (m_stats_running.exchange(true)) {
    return;
  }
  m_stats_thread = std::thread([this]() { stats_loop(); });
}

void EthernetLink::stop_stats_thread() {
  m_stats_running = false;
  if (m_stats_thread.joinable()) {
    m_stats_thread.join();
  }
}

void EthernetLink::stats_loop() {
  while (m_stats_running) {
    update_link_stats();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void EthernetLink::update_link_stats() {
  // WB publishes its own detailed video stats. Never overwrite if WB is active.
  if (openhd::LinkActionHandler::instance().wb_get_supported_channels !=
      nullptr) {
    return;
  }
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  if (m_last_stats_ts_ms == 0) {
    m_last_stats_ts_ms = now_ms;
    m_last_stats_tx_bytes = m_tx_total_bytes.load(std::memory_order_relaxed);
    m_last_stats_tx_packets = m_tx_total_packets.load(std::memory_order_relaxed);
    m_last_stats_rx_bytes = m_rx_total_bytes.load(std::memory_order_relaxed);
    m_last_stats_rx_packets = m_rx_total_packets.load(std::memory_order_relaxed);
    m_last_stats_tx_tele_bytes = m_tx_tele_bytes.load(std::memory_order_relaxed);
    m_last_stats_tx_tele_packets =
        m_tx_tele_packets.load(std::memory_order_relaxed);
    m_last_stats_rx_tele_bytes = m_rx_tele_bytes.load(std::memory_order_relaxed);
    m_last_stats_rx_tele_packets =
        m_rx_tele_packets.load(std::memory_order_relaxed);
    return;
  }
  const int64_t dt_ms = now_ms - m_last_stats_ts_ms;
  if (dt_ms <= 0) {
    return;
  }
  const uint64_t tx_bytes = m_tx_total_bytes.load(std::memory_order_relaxed);
  const uint64_t tx_packets =
      m_tx_total_packets.load(std::memory_order_relaxed);
  const uint64_t rx_bytes = m_rx_total_bytes.load(std::memory_order_relaxed);
  const uint64_t rx_packets =
      m_rx_total_packets.load(std::memory_order_relaxed);
  const uint64_t tx_tele_bytes =
      m_tx_tele_bytes.load(std::memory_order_relaxed);
  const uint64_t tx_tele_packets =
      m_tx_tele_packets.load(std::memory_order_relaxed);
  const uint64_t rx_tele_bytes =
      m_rx_tele_bytes.load(std::memory_order_relaxed);
  const uint64_t rx_tele_packets =
      m_rx_tele_packets.load(std::memory_order_relaxed);

  const uint64_t d_tx_bytes = tx_bytes - m_last_stats_tx_bytes;
  const uint64_t d_tx_packets = tx_packets - m_last_stats_tx_packets;
  const uint64_t d_rx_bytes = rx_bytes - m_last_stats_rx_bytes;
  const uint64_t d_rx_packets = rx_packets - m_last_stats_rx_packets;
  const uint64_t d_tx_tele_bytes = tx_tele_bytes - m_last_stats_tx_tele_bytes;
  const uint64_t d_tx_tele_packets =
      tx_tele_packets - m_last_stats_tx_tele_packets;
  const uint64_t d_rx_tele_bytes = rx_tele_bytes - m_last_stats_rx_tele_bytes;
  const uint64_t d_rx_tele_packets =
      rx_tele_packets - m_last_stats_rx_tele_packets;

  const int64_t scale = 1000;
  const int64_t tx_bps =
      static_cast<int64_t>((d_tx_bytes * 8 * scale) / dt_ms);
  const int64_t rx_bps =
      static_cast<int64_t>((d_rx_bytes * 8 * scale) / dt_ms);
  const int64_t tx_pps =
      static_cast<int64_t>((d_tx_packets * scale) / dt_ms);
  const int64_t rx_pps =
      static_cast<int64_t>((d_rx_packets * scale) / dt_ms);
  const int64_t tx_tele_bps =
      static_cast<int64_t>((d_tx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t rx_tele_bps =
      static_cast<int64_t>((d_rx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t tx_tele_pps =
      static_cast<int64_t>((d_tx_tele_packets * scale) / dt_ms);
  const int64_t rx_tele_pps =
      static_cast<int64_t>((d_rx_tele_packets * scale) / dt_ms);

  m_last_stats_ts_ms = now_ms;
  m_last_stats_tx_bytes = tx_bytes;
  m_last_stats_tx_packets = tx_packets;
  m_last_stats_rx_bytes = rx_bytes;
  m_last_stats_rx_packets = rx_packets;
  m_last_stats_tx_tele_bytes = tx_tele_bytes;
  m_last_stats_tx_tele_packets = tx_tele_packets;
  m_last_stats_rx_tele_bytes = rx_tele_bytes;
  m_last_stats_rx_tele_packets = rx_tele_packets;

  openhd::link_statistics::StatsAirGround stats{};
  stats.is_air = m_profile.is_air;
  stats.ready = true;
  stats.monitor_mode_link.curr_tx_bps = clamp_int32(tx_bps);
  stats.monitor_mode_link.curr_rx_bps = clamp_int32(rx_bps);
  stats.monitor_mode_link.curr_tx_pps = clamp_int16(static_cast<int>(tx_pps));
  stats.monitor_mode_link.curr_rx_pps = clamp_int16(static_cast<int>(rx_pps));
  const int64_t last_rx_ts =
      m_last_rx_packet_ts_ms.load(std::memory_order_relaxed);
  const bool rx_ok = last_rx_ts > 0 && (now_ms - last_rx_ts) <= 5000;
  const auto bitfield = openhd::link_statistics::MonitorModeLinkBitfield{
      false, false, false, rx_ok};
  stats.monitor_mode_link.bitfield =
      openhd::link_statistics::write_monitor_link_bitfield(bitfield);
  stats.telemetry.curr_tx_bps = clamp_int32(tx_tele_bps);
  stats.telemetry.curr_rx_bps = clamp_int32(rx_tele_bps);
  stats.telemetry.curr_tx_pps = clamp_int16(static_cast<int>(tx_tele_pps));
  stats.telemetry.curr_rx_pps = clamp_int16(static_cast<int>(rx_tele_pps));

  if (m_profile.is_air) {
    for (int i = 0; i < openhd::non_wb::VideoBitrateMeter::kMaxStreams; ++i) {
      const auto sample = m_video_bitrate_meter.sample_stream(i, now_ms);
      openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t air_video{};
      const auto cam_stats = openhd::LinkActionHandler::instance().get_cam_info(i);
      if (sample.total_packets == 0 && cam_stats.measured_bitrate_bps == 0) {
        continue;
      }
      air_video.link_index = static_cast<uint8_t>(i);
      air_video.curr_recommended_bitrate =
          cam_stats.target_bitrate_kbits > 0 ? cam_stats.target_bitrate_kbits
                                             : cam_stats.encoding_bitrate_kbits;
      if (cam_stats.measured_bitrate_bps > 0) {
        air_video.curr_measured_encoder_bitrate = static_cast<int32_t>(
            std::min<uint32_t>(cam_stats.measured_bitrate_bps,
                               static_cast<uint32_t>(
                                   std::numeric_limits<int32_t>::max())));
      } else {
        air_video.curr_measured_encoder_bitrate = sample.bitrate_bps;
      }
      air_video.curr_injected_bitrate =
          sample.bitrate_bps > 0 ? sample.bitrate_bps
                                 : air_video.curr_measured_encoder_bitrate;
      air_video.curr_injected_pps = sample.packets_per_second;
      air_video.curr_dropped_frames = 0;
      air_video.curr_fec_percentage = 0;
      stats.stats_wb_video_air.push_back(air_video);
    }
  }
  openhd::LinkActionHandler::instance().update_link_stats(stats);
}
