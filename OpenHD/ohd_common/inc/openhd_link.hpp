//
// Created by consti10 on 12.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_

#include <cstdint>
#include <utility>

#include "openhd_profile.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_video_frame.h"

/**
 * OHDLink refers to "the link" that transmits data from the air unit to the
 * ground unit and vice versa. Since we do not have a dependency between
 * ohd_interface and other modules, we loosely define a interface here (for
 * sending data and registering a callback for receiving data) It hides away the
 * underlying implementation (e.g. wifibroadcast aka wifi cards in monitor mode
 * or lte or ... ) However, r.n the only existing implementation is
 * wifibroadcast. What a openhd link MUST support to integrate is well defined:
 * 1) Send telemetry data from air to ground and vice versa
 *  => 1 bidirectional (aka air to ground and ground to air) but (recommended)
 * lossy (since mavlink deals with packet loss / retransmissions /) link 2) Send
 * video data from air to ground, recommended 2 instances (primary and secondary
 * video), at least 1 required
 *  => 2x unidirectional (recommended lossy, but FEC protected) links for
 * primary and secondary video from air to ground
 *
 *  In general, there should be exactly one instance of ohd_link on the air unit
 * and one on the ground unit.
 */
class OHDLink {
 public:
  typedef std::function<void(std::shared_ptr<std::vector<uint8_t>> data)>
      ON_TELE_DATA_CB;

 public:
  // --- Telemetry air and ground both receive and send --------
  // Telemetry TX special - retransmission(s) - duplicate specific mavlink
  // messages that are going from gnd to air (or vice versa) to increase
  // reliability.
  struct TelemetryTxPacket {
    std::shared_ptr<std::vector<uint8_t>> data;
    int n_injections = 1;
  };
  /**
   * valid on both air and ground instance
   * send telemetry data to the ground if air unit and vice versa.
   */
  virtual void transmit_telemetry_data(TelemetryTxPacket packet) = 0;

  /**
   * valid on both air and ground instance
   * called every time telemetry data is received - used by ohd_telemetry to
   * react to incoming packets
   * @param data the received message
   */
  void on_receive_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data) {
    auto tmp = m_tele_data_cb;
    if (tmp) {
      auto& cb = *tmp;
      cb(std::move(data));
    }
  }
  void register_on_receive_telemetry_data_cb(const ON_TELE_DATA_CB& cb) {
    if (cb == nullptr) {
      m_tele_data_cb = nullptr;
      return;
    }
    m_tele_data_cb = std::make_shared<ON_TELE_DATA_CB>(cb);
  }

 public:
  typedef std::function<void(int stream_index, const uint8_t* data,
                             int data_len)>
      ON_VIDEO_DATA_CB;
  // -------- video, air sends, ground receives ---------
  /**
   * Video, unidirectional
   * only valid on air (transmit)
   * @param stream_index 0 -> primary video stream, 1 -> secondary video stream
   * @param fragmented_video_frame the "frame" to transmit
   */
  virtual void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) = 0;

  // Called by the wifibroadcast receiver on the ground unit only
  void on_receive_video_data(int stream_index, const uint8_t* data,
                             int data_len) {
    auto tmp = m_video_data_cb;
    if (tmp) {
      auto& cb = *tmp;
      cb(stream_index, data, data_len);
    }
  }
  void register_on_receive_video_data_cb(const ON_VIDEO_DATA_CB& cb) {
    if (cb == nullptr) {
      m_video_data_cb = nullptr;
      return;
    }
    m_video_data_cb = std::make_shared<ON_VIDEO_DATA_CB>(cb);
  }

 private:
  std::shared_ptr<ON_TELE_DATA_CB> m_tele_data_cb;
  std::shared_ptr<ON_VIDEO_DATA_CB> m_video_data_cb;

 public:
  typedef std::function<void(const uint8_t* data, int data_len)>
      ON_AUDIO_DATA_RX_CB;
  ON_AUDIO_DATA_RX_CB m_audio_data_rx_cb = nullptr;
  virtual void transmit_audio_data(const openhd::AudioPacket& audio_packet) = 0;
  void on_receive_audio_data(const uint8_t* data, int data_len) {
    if (m_audio_data_rx_cb) {
      m_audio_data_rx_cb(data, data_len);
    }
  }
};

class DummyDebugLink : public OHDLink {
 public:
  openhd::ON_ENCODE_FRAME_CB m_opt_frame_cb = nullptr;
  explicit DummyDebugLink() {
    m_console_tele = openhd::log::create_or_get("tele");
    m_console_video = openhd::log::create_or_get("video");
    m_console_audio = openhd::log::create_or_get("audio");
  }
  void transmit_telemetry_data(TelemetryTxPacket packet) override {
    m_console_tele->debug("Got {} telemetry fragments", packet.data->size());
  }
  // Called by the camera stream on the air unit only
  // transmit video data via wifibradcast
  void transmit_video_data(
      int stream_index,
      const openhd::FragmentedVideoFrame& fragmented_video_frame) override {
    int64_t total_bytes = 0;
    for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
      total_bytes += fragment->size();
    }
    m_console_video->debug("Got Frame. Fragments:{} total: {}Bytes",
                           fragmented_video_frame.rtp_fragments.size(),
                           total_bytes);
    if (m_opt_frame_cb) {
      m_opt_frame_cb(stream_index, fragmented_video_frame);
    }
  }
  void transmit_audio_data(const openhd::AudioPacket& audio_packet) override {
    m_console_audio->debug("Got audio data {}", audio_packet.data->size());
  }

 private:
  std::shared_ptr<spdlog::logger> m_console_video;
  std::shared_ptr<spdlog::logger> m_console_tele;
  std::shared_ptr<spdlog::logger> m_console_audio;
};

#endif  // OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_
