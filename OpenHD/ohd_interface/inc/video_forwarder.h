//
// Created by consti10 on 09.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VIDEO_FORWARDER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VIDEO_FORWARDER_H_

#include "../../lib/wifibroadcast/src/UdpWBReceiver.hpp"

// Video data received from wifibroadcast is made available via UDP on the ground unit.
// Localhost 5600 is always on by default, other client(s) can be added / removed dynamically.
// Note that on the ground, at least for now (as long as we don't change anything in this regard)
// The video stream data from the wifibroadcast rx instance is directly made available to UDP localhost
// without any modification(s) - it is already in RTP format. Note that it is protected by FEC, but no guarantees about the
// data are made - the displaying application (e.g. QOpenHD) needs to deal with possible packet loss.
// TODO: R.n it is not needed, but with even more clients we might have to use multithreading here.
class GroundVideoForwarder{
 public:
  GroundVideoForwarder();
  // Start forwarding to another ip::port
  void addForwarder(std::string client_addr, int client_udp_port);
  // stop forwarding to ip::port
  // safe to call if not already forwarding to ip::port
  void removeForwarder(std::string client_addr, int client_udp_port);
  // called by the wb receiver
  void forward_data(int stream_idx,const uint8_t* data, int data_len);
 private:
   std::unique_ptr<SocketHelper::UDPMultiForwarder> udpMultiForwarder;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_VIDEO_FORWARDER_H_
