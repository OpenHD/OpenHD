//
// Created by consti10 on 04.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_

#include "../../lib/wifibroadcast/src/HelperSources/SocketHelper.hpp"
#include "ohd_link.hpp"
#include "openhd-external-device.hpp"

// The ground just stupidly forwards video (rtp fragments, to be exact) via UDP
// for QOpenHD and/or more device(s) to decode and display.
// It does not touch the video data in any way (other than wb and its FEC protection).
// re-fragmentation is up to the displaying application (which is why we have rtp ;) )
// NOTE: There is no way to query any information or change camera/streaming info on the ground. This design is by purpose !

// Video data received from wifibroadcast is made available via UDP on the ground unit.
// Localhost 5600 is always on by default, other client(s) can be added / removed dynamically.
// Note that on the ground, at least for now (as long as we don't change anything in this regard)
// The video stream data from the wifibroadcast rx instance is directly made available to UDP localhost
// without any modification(s) - it is already in RTP format. Note that it is protected by FEC, but no guarantees about the
// data are made - the displaying application (e.g. QOpenHD) needs to deal with possible packet loss.
class OHDVideoGround{
 public:
  /**
   * Forward primary and secondary video data
   * @param link_handle where we get the data that needs to be forwarded from
   */
  explicit OHDVideoGround(std::shared_ptr<OHDLink> link_handle);
  ~OHDVideoGround();
  //
  void set_ext_devices_manager(std::shared_ptr<openhd::ExternalDeviceManager> ext_device_manager);
 private:
  // Start forwarding to another ip
  void addForwarder(const std::string& client_addr);
  // stop forwarding to ip
  void removeForwarder(const std::string& client_addr);
 private:
  std::shared_ptr<OHDLink> m_link_handle;
  std::unique_ptr<SocketHelper::UDPMultiForwarder> m_primary_video_forwarder;
  std::unique_ptr<SocketHelper::UDPMultiForwarder> m_secondary_video_forwarder;
  /**
   * Forward video to all device(s) consuming video.
   * Called by the ohd link handle (aka only wb right now)
   * @param stream_index 0 for primary video stream, 1 for secondary, ...
   * @param data and @param data_len: r.n always a full rtp frame fragment
   */
  void on_video_data(int stream_index,const uint8_t * data,int data_len);
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OHD_VIDEO_GROUND_H_
