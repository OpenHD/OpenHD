//
// Created by consti10 on 03.05.22.
//
#include <openhd-global-constants.hpp>
#include <utility>

#include "gstreamerstream.h"
#include "ohd_video_air.h"

OHDVideoAir::OHDVideoAir(OHDPlatform platform1,std::vector<Camera> cameras,
                   std::shared_ptr<openhd::ActionHandler> opt_action_handler,
                   std::shared_ptr<OHDLink> link)
    : m_platform(platform1),
      m_opt_action_handler(std::move(opt_action_handler)),
      m_link_handle(std::move(link))
{
  m_console = openhd::log::create_or_get("v_air");
  assert(m_console);
  assert(!cameras.empty());
  m_console->debug("OHDVideo::OHDVideo()");
  if(cameras.size()>MAX_N_CAMERAS){
    m_console->warn("More than {} cameras, dropping cameras",MAX_N_CAMERAS);
    cameras.resize(MAX_N_CAMERAS);
  }
  m_generic_settings=std::make_unique<AirCameraGenericSettingsHolder>();
  if(m_generic_settings->get_settings().switch_primary_and_secondary && cameras.size()==2){
    // swap cam 1 and cam 2 (primary and secondary) - aka if they are detected in the wrong order
    auto cam1=cameras.at(1);
    auto cam2=cameras.at(0);
    cam1.index=0;
    cam2.index=1;
    cameras.resize(0);
    cameras.push_back(cam1);
    cameras.push_back(cam2);
  }
  std::vector<std::shared_ptr<CameraHolder>> camera_holders;
  for(const auto& camera:cameras){
    camera_holders.emplace_back(std::make_unique<CameraHolder>(camera,m_opt_action_handler));
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

std::string OHDVideoAir::createDebug() const {
  // TODO make it much more verbose
  std::stringstream ss;
  ss << "OHDVideo::N camera streams:" << m_camera_streams.size() << "\n";
  for (int i = 0; i < m_camera_streams.size(); i++) {
    const auto &stream = m_camera_streams.at(i);
    ss << "Camera stream:" << i << stream->createDebug() << "\n";
  }
  return ss.str();
}

void OHDVideoAir::configure(const std::shared_ptr<CameraHolder>& camera_holder) {
  const auto camera=camera_holder->get_camera();
  m_console->debug("Configuring camera:"+camera_type_to_string(camera.type));
  // R.N we use gstreamer for pretty much everything
  // But this might change in the future
  switch (camera.type) {
    case CameraType::RPI_VEYE_CSI_V4l2:
    case CameraType::RPI_CSI_MMAL:
    case CameraType::JETSON_CSI:
    case CameraType::IP:
    case CameraType::ROCKCHIP_CSI:
    case CameraType::UVC:
    case CameraType::ROCKCHIP_HDMI:
    case CameraType::CUSTOM_UNMANAGED_CAMERA:
    case CameraType::DUMMY_SW: {
      m_console->debug("GStreamerStream for Camera index:{}",camera.index);
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder,m_link_handle);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    case CameraType::RPI_CSI_LIBCAMERA: {
      m_console->debug("LibCamera index:{}", camera.index);
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder, m_link_handle);
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

void OHDVideoAir::restartIfStopped() {
  for(auto& stream:m_camera_streams){
    stream->restartIfStopped();
  }
}

std::vector<std::shared_ptr<openhd::ISettingsComponent>>
OHDVideoAir::get_all_camera_settings() {
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> ret;
  for(auto& stream: m_camera_streams){
    ret.push_back(stream->m_camera_holder);
  }
  return ret;
}

void OHDVideoAir::handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) {
  for(auto& stream:m_camera_streams){
    stream->handle_change_bitrate_request(lb);
  }
}

std::vector<openhd::Setting> OHDVideoAir::get_generic_settings() {
  std::vector<openhd::Setting> ret;
  // N of discovered cameras, for debugging
  const auto n_cameras=static_cast<int>(m_camera_streams.size());
  ret.push_back(openhd::create_read_only_int("V_N_CAMERAS",n_cameras));
  if(n_cameras>1){
    auto cb_switch_primary_and_secondary=[this](std::string,int value){
      if(!openhd::validate_yes_or_no(value))return false;
      m_generic_settings->unsafe_get_settings().switch_primary_and_secondary=value;
      m_generic_settings->persist();
      // Do nothing, switch requires reboot
      return true;
    };
    ret.push_back(openhd::Setting{"V_SWITCH_CAM",openhd::IntSetting{m_generic_settings->unsafe_get_settings().switch_primary_and_secondary,cb_switch_primary_and_secondary}});
  }
  return ret;
}
