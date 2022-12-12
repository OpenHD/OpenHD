//
// Created by consti10 on 22.11.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_

#include "openhd-video-transmit-interface.h"
#include "openhd-telemetry-tx-rx.h"

#include <cstdint>
#include <utility>
#include "openhd-profile.hpp"

/**
 * TODO migrate
 * OHDLink refers to the "link that transmits data from/to the air unit to the ground unit".
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
  explicit OHDLink(OHDProfile profile): m_profile(std::move(profile)){
    m_tx_rx_handle=std::make_shared<openhd::TxRxTelemetry>();
  }
 public:
  // Telemetry, bidirectional (receive and transmit each)

  /**
   * valid on both air and ground instance
   * send telemetry data to the ground if air unit and vice versa.
   */
  virtual bool transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)=0;

  /**
   * valid on both air and ground instance
   * called every time telemetry data is received - used by ohd_telemetry to react to incoming packets
   * @param data the received message
   */
  virtual void on_receive_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data) {
    auto tmp=m_tx_rx_handle;
    if(tmp){
      tmp->forward_to_on_receive_cb_if_set(data);
    }
  }
 public:
  /**
   * Video, unidirectional
   * only valid on air (transmit)
   * @param stream_index 0 -> primary video stream, 1 -> secondary video stream
   * @param fragmented_video_frame the "frame" to transmit
   */
  virtual void transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame)=0;

  /**
   * For registering the callback called with the received telemetry packets.
   */
  std::shared_ptr<openhd::TxRxTelemetry> get_telemetry_tx_rx_handle(){
    return m_tx_rx_handle;
  }
 protected:
  const OHDProfile m_profile;
 private:
  std::shared_ptr<openhd::TxRxTelemetry> m_tx_rx_handle;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_
