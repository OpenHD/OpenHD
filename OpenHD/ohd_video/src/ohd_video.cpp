//
// Created by consti10 on 03.05.22.
//
#include <openhd-global-constants.hpp>
#include <utility>

#include "gstreamerstream.h"
#include "ohd_video.h"
//#include "veyestream.h"

OHDVideo::OHDVideo(OHDPlatform platform1,const std::vector<Camera>& cameras,
                   std::shared_ptr<openhd::ActionHandler> opt_action_handler,
                   std::shared_ptr<openhd::ITransmitVideo> interface_transmit_video) :
	m_platform(platform1),m_opt_action_handler(std::move(opt_action_handler)),
        m_interface_transmit_video(std::move(interface_transmit_video))
{
  m_console = openhd::log::create_or_get("video");
  assert(m_console);
  assert(!cameras.empty());
  m_console->debug("OHDVideo::OHDVideo()");
  std::vector<std::shared_ptr<CameraHolder>> camera_holders;
  for(const auto& camera:cameras){
    if(camera_holders.size()<MAX_N_CAMERAS){
      camera_holders.emplace_back(std::make_unique<CameraHolder>(camera,m_opt_action_handler));
    }else{
      m_console->warn("Dropping camera {}, too many cameras",camera.to_string());
    }
  }
  startup_fix_common_issues(camera_holders);
  assert(camera_holders.size()<=MAX_N_CAMERAS);
  for (auto &camera: camera_holders) {
    configure(camera);
  }
  if(m_opt_action_handler){
    m_opt_action_handler->action_request_bitrate_change_register([this](openhd::ActionHandler::LinkBitrateInformation lb){
      this->handle_change_bitrate_request(lb);
    });
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

void OHDVideo::configure(const std::shared_ptr<CameraHolder>& camera_holder) {
  const auto camera=camera_holder->get_camera();
  m_console->debug("Configuring camera:"+camera_type_to_string(camera.type));
  // R.N we use gstreamer only for everything except veye
  // (veye also uses gstreamer, but we do not launch it via gst-launch)
  switch (camera.type) {
    case CameraType::RPI_VEYE_CSI_MMAL:{
      /*m_console->debug("VEYE stream for Camera index:{}",camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<VEYEStream>(m_platform.platform_type, camera_holder, udp_port);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;*/
      m_console->error("Veye had to be dropped in 2.2.4");
      break;
    }
    case CameraType::RPI_CSI_MMAL:
    case CameraType::JETSON_CSI:
    case CameraType::IP:
    case CameraType::ROCKCHIP_CSI:
    case CameraType::UVC:
    case CameraType::ROCKCHIP_HDMI:
    case CameraType::CUSTOM_UNMANAGED_CAMERA:
    case CameraType::DUMMY_SW: {
      m_console->debug("GStreamerStream for Camera index:{}",camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder,m_interface_transmit_video);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    case CameraType::RPI_CSI_LIBCAMERA: {
      m_console->debug("LibCamera index:{}", camera.index);
      const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder, m_interface_transmit_video);
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
    ret.push_back(stream->m_camera_holder);
  }
  return ret;
}

void OHDVideo::handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) {
  for(auto& stream:m_camera_streams){
    stream->handle_change_bitrate_request(lb);
  }
}
