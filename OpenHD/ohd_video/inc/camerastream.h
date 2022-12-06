#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <string>
#include <vector>

#include "camera_holder.hpp"
#include "openhd-platform.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-video-transmit-interface.h"

/**
 * Every camera stream should inherit from this class.
 * This hides away the underlying implementation (for example gstreamer,...) for
 * different platform(s). The paradigms developers should aim for with each
 * camera stream are:
 * 1) Once an instance is created, it will start generating video data, already encoded and packetized with respect to the link MTU.
 * RTP MUST be used for packetization (at least for now)
 * 2) If the camera disconnects or the underlying process crashes (for whatever reason) the underlying
 * implementation should re-start the camera and encoding process
 * 3) If the user changes camera parameters, it should store these changes locally (such that
 * they are also set after the next re-start) and apply the changes. It is no
 * problem to just restart the underlying camera/encoding process with the new
 * parameters.
 * 4) The implementation(s) should handle the differences between camera(s) in regards to supported and not supported parameters
 *
 * Video streaming in OpenHD is always unidirectional and lossy (FEC). However, this is done by the link implementation - here we only
 * generate encoded data and packetize it into rtp fragments, then forward it.
 */
class CameraStream {
 public:
  /**
   * After a camera stream is constructed, it won't start streaming until
   * setup() and start() are called
   * @param platform the platform we are running on
   * @param camera_holder the camera to create the stream with, camera_holder provides access to the camera (capabilities) and settings.
   * @param video_udp_port the udp port where rtp data is forwarded to, must
   * match with interface in OpenHD
   */
  CameraStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder, uint16_t video_udp_port);
  CameraStream(PlatformType platform_type,std::shared_ptr<CameraHolder> camera_holder,std::shared_ptr<openhd::ITransmitVideo> itransmit);

  // It is a good common programming practice to make them pure virtual
  // setup everything needed to start streaming
  virtual void setup() = 0;
  // start streaming
  virtual void start() = 0;
  // stop streaming
  virtual void stop() = 0;
  /**
   * Create a verbose debug string about the current state of the stream.
   * @return a string, can be printed to stdout or similar.
   */
  [[nodiscard]] virtual std::string createDebug() = 0;

  /**
   * This can be called in regular intervals by the main OpenHD thread to
   * restart a camera stream if it has stopped / crashed for some reason.
   */
  virtual void restartIfStopped() = 0;
  /**
   * Handle a change in the bitrate, most likely requested by the RF link.
   * This is the only value an implementation should support changing without a complete restart of the pipeline /
   * stream. It is okay to not implement this interface method properly, e.g leave it empty.
   */
   virtual void handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb)=0;
 public:
  std::shared_ptr<CameraHolder> m_camera_holder;
 protected:
  const PlatformType m_platform_type;
  // This is the UDP port the video (for now rtp) stream is send to.
  // It then needs to be picked up, most likely by a wfb instance created by
  // ohd-interface
  const uint16_t m_video_udp_port;
  std::shared_ptr<openhd::ITransmitVideo> m_transmit_interface= nullptr;
};

#endif
