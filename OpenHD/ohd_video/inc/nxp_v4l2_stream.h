#ifndef OPENHD_NXP_V4L2_STREAM_H
#define OPENHD_NXP_V4L2_STREAM_H

#include <memory>

#include "camerastream.h"

// Native i.MX8 capture and VPU encoding. With the Hantro SDK, ORQA cameras
// share capture/encoder memory through the direct VC8000E API. Other NXP
// cameras retain the NV12M V4L2 mem2mem path. Both use OpenHD's RTPHelper.
class NxpV4l2Stream final : public CameraStream {
 public:
  NxpV4l2Stream(std::shared_ptr<CameraHolder> camera_holder,
                openhd::ON_ENCODE_FRAME_CB out_cb);
  ~NxpV4l2Stream();

  void start_looping() override;
  void terminate_looping() override;
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  void handle_update_arming_state(bool armed) override;

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif  // OPENHD_NXP_V4L2_STREAM_H
