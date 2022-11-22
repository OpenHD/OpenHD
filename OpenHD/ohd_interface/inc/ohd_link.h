//
// Created by consti10 on 22.11.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_

#include <cstdint>
#include "openhd-profile.hpp"

/**
 * OHDLink refers to the "link that transmits data from/to the air unit to the ground unit".
 * It hides away the underlying implementation (e.g. wifibroadcast aka wifi cards in monitor mode or lte or ... )
 * However, r.n the only existing implementation is wifibroadcast.
 * What a openhd link MUST support to integrate with openhd is well defined:
 * 1) Send telemetry data from air to ground and vice versa
 *  => 1 bidirectional (aka air to ground and ground to air) but (recommended) lossy (since mavlink deals with packet loss / retransmissions /) link
 * 2) Send video data from air to ground, recommended 2 instances (primary and secondary video), at least 1 required
 *  => 2x unidirectional (recommended lossy) links for primary and secondary video from air to ground
 *
 *  In general, there should be exactly one instance of ohd_link on the air unit and one on the ground unit.
 */
class OHDLink{
 public:
  // Telemetry, bidirectional (receive and transmit each)
  // valid on both air and ground instance
  // send telemetry data to the ground if air unit and vice versa.
  virtual bool transmit_telemetry_data(const uint8_t* data,int data_len)=0;

  // valid on both air and ground instance
  // called every time telemetry data is received
  virtual void on_receive_telemetry_data(const uint8_t* data,int data_len)=0;
 public:
  // Video, unidirectional
  // only valid on air (transmit)
  virtual bool transmit_video_data_primary(const uint8_t* data,int data_len)=0;
  virtual bool transmit_video_data_secondary(const uint8_t* data,int data_len)=0;

  // only valid on ground (receive)
  virtual bool on_receive_video_data_primary(const uint8_t* data,int data_len)=0;
  virtual bool on_receive_video_data_secondary(const uint8_t* data,int data_len)=0;

 protected:
  const OHDProfile m_profile;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_OHD_LINK_H_
