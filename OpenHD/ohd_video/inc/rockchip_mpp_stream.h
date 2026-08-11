#ifndef OPENHD_ROCKCHIP_MPP_STREAM_H
#define OPENHD_ROCKCHIP_MPP_STREAM_H

#include <atomic>
#include <memory>
#include <thread>

#include "camerastream.h"

// RV1126(B) stream using GStreamer only for raw NV12 capture and Rockchip MPP
// directly for encoding. MPP details live in the implementation so this header
// remains usable by builds which do not have the Rockchip SDK installed.
class RockchipMppStream final : public CameraStream {
 public:
  RockchipMppStream(std::shared_ptr<CameraHolder> camera_holder,
                    openhd::ON_ENCODE_FRAME_CB out_cb);
  ~RockchipMppStream();

  void start_looping() override;
  void terminate_looping() override;
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  void handle_update_arming_state(bool armed) override;

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif
