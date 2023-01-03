//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_

#include "../../lib/wifibroadcast/src/HelperSources/SocketHelper.hpp"
#include "ohd_link.hpp"

// The ground just stupidly forwards video (rtp fragments, to be exact) via UDP
// for QOpenHD and/or more device(s) to decode and display.
// NOTE: There is no way to query any information or change camera/streaming info on the ground. This design is by purpose !

// Video data received from wifibroadcast is made available via UDP on the ground unit.
// Localhost 5600 is always on by default, other client(s) can be added / removed dynamically.
// Note that on the ground, at least for now (as long as we don't change anything in this regard)
// The video stream data from the wifibroadcast rx instance is directly made available to UDP localhost
// without any modification(s) - it is already in RTP format. Note that it is protected by FEC, but no guarantees about the
// data are made - the displaying application (e.g. QOpenHD) needs to deal with possible packet loss.
class OHDVideoGround{
 public:
  explicit OHDVideoGround(std::shared_ptr<OHDLink> link_handle);
  ~OHDVideoGround();
  //
  // Start forwarding to another ip::port
  void addForwarder(std::string client_addr, int client_udp_port);
  // stop forwarding to ip::port
  // safe to call if not already forwarding to ip::port
  void removeForwarder(std::string client_addr, int client_udp_port);
 private:
  std::shared_ptr<OHDLink> m_link_handle;
  std::unique_ptr<SocketHelper::UDPMultiForwarder> udpMultiForwarder;
  /**
   * Forward video to all device(s) consuming video.
   * Called by the ohd link handle (aka only wb right now)
   * @param stream_index 0 for primary video stream, 1 for secondary, ...
   * @param data and @param data_len: r.n always a full rtp frame fragment
   */
  void on_video_data(int stream_index,const uint8_t * data,int data_len);
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
