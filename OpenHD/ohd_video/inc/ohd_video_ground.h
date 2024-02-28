//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_

#include "openhd_external_device.h"
#include "openhd_link.hpp"
#include "openhd_udp.h"

// The ground just stupidly forwards video (rtp fragments, to be exact) via UDP
// for QOpenHD and/or more device(s) to decode and display.
// It does not touch the video data in any way (other than wb and its FEC
// protection, but that currently happens in the wifibroadcast namespace).
// re-fragmentation is up to the displaying application (which is why we have
// rtp ;) ) NOTE: There is no way to query any information or change
// camera/streaming info on the ground. This design is by purpose !
class OHDVideoGround {
 public:
  /**
   * Forward primary and secondary video data
   * @param link_handle where we get the data that needs to be forwarded from
   */
  explicit OHDVideoGround(std::shared_ptr<OHDLink> link_handle);
  ~OHDVideoGround();

 private:
  // Start forwarding to another ip
  void addForwarder(const std::string& client_addr);
  // stop forwarding to ip
  void removeForwarder(const std::string& client_addr);

 private:
  std::shared_ptr<spdlog::logger> m_console;
  std::shared_ptr<OHDLink> m_link_handle;
  std::unique_ptr<openhd::UDPMultiForwarder> m_primary_video_forwarder;
  std::unique_ptr<openhd::UDPMultiForwarder> m_secondary_video_forwarder;
  std::unique_ptr<openhd::UDPMultiForwarder> m_audio_forwarder;
  /**
   * Forward video to all device(s) consuming video.
   * Called by the ohd link handle (aka only wb right now)
   * @param stream_index 0 for primary video stream, 1 for secondary, ...
   * @param data and @param data_len: r.n always a full rtp frame fragment
   */
  void on_video_data(int stream_index, const uint8_t* data, int data_len);

  /**
   * Forward audio. We only have up to 1 audio stream
   */
  void on_audio_data(const uint8_t* data, int data_len);

 private:
  void start_stop_forwarding_external_device(
      openhd::ExternalDevice external_device, bool connected);
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
