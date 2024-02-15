#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <string>
#include <vector>

#include "camera_holder.h"
#include "openhd_action_handler.h"
#include "openhd_platform.h"
#include "openhd_video_frame.h"

/**
 * Every camera stream should inherit from this class.
 * This hides away the underlying implementation (for example gstreamer,...) for
 * different platform(s). The paradigms developers should aim for with each
 * camera stream are:
 * 1) Once an instance is created, it will start generating video data, already
 * encoded and packetized with respect to the link MTU. RTP MUST be used for
 * packetization (at least for now) 2) If the camera disconnects or the
 * underlying process crashes (for whatever reason) the underlying
 * implementation should re-start the camera and encoding process
 * 3) If the user changes camera parameters, it should store these changes
 * locally (such that they are also set after the next re-start) and apply the
 * changes. It is no problem to just restart the underlying camera/encoding
 * process with the new parameters. 4) The implementation(s) should handle the
 * differences between camera(s) in regards to supported and not supported
 * parameters
 *
 * Video streaming in OpenHD is always unidirectional and lossy (FEC). However,
 * this is done by the link implementation - here we only generate encoded data
 * and packetize it into rtp fragments, then forward it.
 */
class CameraStream {
 public:
  /**
   * After a camera stream is constructed, it won't start streaming until
   * setup() and start() are called
   * @param platform the platform we are running on
   * @param camera_holder the camera to create the stream with, camera_holder
   * provides access to the camera (capabilities) and settings.
   * @param i_transmit abstract interface where encoded video data is forwarded
   * to (was UDP port previously)
   */
  CameraStream(std::shared_ptr<CameraHolder> camera_holder,
               openhd::ON_ENCODE_FRAME_CB out_cb);
  CameraStream(const CameraStream&) = delete;
  CameraStream(const CameraStream&&) = delete;

  // after start_looping is called the camera should start streaming (generating
  // video data) as soon as possible terminate_loping() is called when openhd
  // terminates (only for development) The camera is responsible to implement
  // its loop thread such that it can react to setting changes
  virtual void start_looping() = 0;
  virtual void terminate_looping() = 0;
  /**
   * Handle a change in the bitrate, most likely requested by the RF link.
   * This is the only value an implementation should support changing without a
   * complete restart of the pipeline / stream. It is okay to not implement this
   * interface method properly, e.g leave it empty.
   */
  virtual void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) = 0;
  /**
   * Handle a change in the arming state
   * We have air video recording depending on the arming state, but the setting
   * and implementation is camera specific. It is okay to not implement this
   * interface method properly, e.g leave it empty.
   */
  virtual void handle_update_arming_state(bool armed) = 0;

 public:
  std::shared_ptr<CameraHolder> m_camera_holder;
  static constexpr auto CAM_STATUS_STREAMING = 1;
  static constexpr auto CAM_STATUS_RESTARTING = 2;

 protected:
  openhd::ON_ENCODE_FRAME_CB m_output_cb;
};

#endif
