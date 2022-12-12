//
// Created by consti10 on 12.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_

#include <cstdint>
#include <utility>
#include "openhd-profile.hpp"
#include "openhd-video-frame.h"


/**
 * OHDLink refers to "the link" that transmits data from/to the air unit to the ground unit".
 * Since we do not have a dependency between ohd_interface and other modules, we loosely define a interface here
 * (for sending data and registering a callback for receiving data)
 * It hides away the underlying implementation (e.g. wifibroadcast aka wifi cards in monitor mode or lte or ... )
 * However, r.n the only existing implementation is wifibroadcast.
 * What a openhd link MUST support to integrate is well defined:
 * 1) Send telemetry data from air to ground and vice versa
 *  => 1 bidirectional (aka air to ground and ground to air) but (recommended) lossy (since mavlink deals with packet loss / retransmissions /) link
 * 2) Send video data from air to ground, recommended 2 instances (primary and secondary video), at least 1 required
 *  => 2x unidirectional (recommended lossy, but FEC protected) links for primary and secondary video from air to ground
 *
 *  In general, there should be exactly one instance of ohd_link on the air unit and one on the ground unit.
 */
class OHDLink{
 public:
  typedef std::function<void(std::shared_ptr<std::vector<uint8_t>> data)> ON_TELE_DATA_CB;
 public:
  // --- Telemetry air and ground both receive and send --------
  /**
   * valid on both air and ground instance
   * send telemetry data to the ground if air unit and vice versa.
   */
  virtual void transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)=0;

  /**
   * valid on both air and ground instance
   * called every time telemetry data is received - used by ohd_telemetry to react to incoming packets
   * @param data the received message
   */
  void on_receive_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data) {
    auto tmp=m_tele_data_cb;
    if(tmp){
      auto& cb=*tmp;
      cb(std::move(data));
    }
  }
  void register_on_receive_telemetry_data_cb(ON_TELE_DATA_CB cb){
    if(cb== nullptr){
      m_tele_data_cb= nullptr;
      return;
    }
    m_tele_data_cb=std::make_shared<ON_TELE_DATA_CB>(cb);
  }
 public:
  // -------- video, air sends, ground receives ---------
  /**
   * Video, unidirectional
   * only valid on air (transmit)
   * @param stream_index 0 -> primary video stream, 1 -> secondary video stream
   * @param fragmented_video_frame the "frame" to transmit
   */
  virtual void transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame)=0;

 private:
  std::shared_ptr<ON_TELE_DATA_CB> m_tele_data_cb;
};

#endif  // OPENHD_OPENHD_OHD_COMMON_OHD_LINK_HPP_
