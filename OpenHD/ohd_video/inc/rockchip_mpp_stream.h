#ifndef OPENHD_ROCKCHIP_MPP_STREAM_H
#define OPENHD_ROCKCHIP_MPP_STREAM_H

#include <atomic>
#include <memory>
#include <thread>

#include "camerastream.h"

// Native Rockchip camera-worker interface. RV1126(B) uses V4L2 + MPP;
// RV1103/RV1106 builds use RKAIQ + Rockit VI/VENC. Neither requires GStreamer.
// SDK details stay in the platform-specific implementation.
class RockchipMppStream final : public CameraStream {
 public:
  RockchipMppStream(std::shared_ptr<CameraHolder> camera_holder,
                    openhd::ON_ENCODE_FRAME_CB out_cb);
  ~RockchipMppStream();

  void start_looping() override;
  void terminate_looping() override;
  void request_keyframe();
  void set_qp_limits(int minimum, int maximum);
  void set_orientation(int rotation_degree, int flip);
  int recording_setting() const;
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  void handle_update_arming_state(bool armed) override;

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif
