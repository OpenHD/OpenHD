//
// Created by consti10 on 03.05.22.
//
#include <openhd-global-constants.hpp>

#include "gstreamerstream.h"
#include "ohd_video.h"
#include "veyestream.h"

OHDVideo::OHDVideo(OHDPlatform platform1,DiscoveredCameraList cameras,std::shared_ptr<openhd::ActionHandler> opt_action_handler) :
	platform(platform1) {
  m_console = openhd::log::create_or_get("video");
  assert(m_console);
  assert(!cameras.empty());
  m_console->debug("OHDVideo::OHDVideo()");
  std::vector<std::shared_ptr<CameraHolder>> camera_holders;
  for(const auto& camera:cameras){
    camera_holders.emplace_back(std::make_unique<CameraHolder>(camera,opt_action_handler));
  }
  for (auto &camera: camera_holders) {
	configure(camera);
  }
  m_console->debug( "OHDVideo::running");
}

std::string OHDVideo::createDebug() const {
  // TODO make it much more verbose
  std::stringstream ss;
  ss << "OHDVideo::N camera streams:" << m_camera_streams.size() << "\n";
  for (int i = 0; i < m_camera_streams.size(); i++) {
    const auto &stream = m_camera_streams.at(i);
    ss << "Camera stream:" << i << stream->createDebug() << "\n";
  }
  return ss.str();
}

void OHDVideo::configure(std::shared_ptr<CameraHolder> camera_holder) {
  const auto camera=camera_holder->get_camera();
  m_console->debug("Configuring camera:"+camera_type_to_string(camera.type));
  // R.N we use gstreamer only for everything except veye
  // (veye also uses gstreamer, but we do not launch it via gst-launch)
  switch (camera.type) {
    case CameraType::RaspberryPiVEYE:{
      m_console->debug("VEYE stream for Camera index:{}",camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<VEYEStream>(platform.platform_type, camera_holder, udp_port);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    case CameraType::RaspberryPiCSI:
    case CameraType::JetsonCSI:
    case CameraType::IP:
    case CameraType::RockchipCSI:
    case CameraType::UVC:
    case CameraType::RockchipHDMI:
    case CameraType::Dummy: {
      m_console->debug("GStreamerStream for Camera index:{}",camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<GStreamerStream>(platform.platform_type, camera_holder, udp_port);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    case CameraType::Libcamera: {
      m_console->debug("LibCamera index:{}", camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<GStreamerStream>(platform.platform_type, camera_holder, udp_port);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    default: {
      m_console->error("Unknown camera type, skipping");
    }
  }
}

void OHDVideo::restartIfStopped() {
  for(auto& stream:m_camera_streams){
    stream->restartIfStopped();
  }
}

std::vector<std::shared_ptr<openhd::ISettingsComponent>> OHDVideo::get_setting_components() {
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> ret;
  for(auto& stream: m_camera_streams){
    ret.push_back(stream->_camera_holder);
  }
  return ret;
}


