//
// Created by consti10 on 03.05.22.
//
//#include <openhd-global-constants.hpp>
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
  if(m_platform.platform_type==PlatformType::RaspberryPi){
    m_rpi_os_change_config_handler=std::make_unique<openhd::rpi::os::ConfigChangeHandler>(m_platform);
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
    case CameraType::ALLWINNER_CSI:
    case CameraType::UVC:
    case CameraType::ROCKCHIP_HDMI:
    case CameraType::CUSTOM_UNMANAGED_CAMERA:
    case CameraType::DUMMY_SW: {
      m_console->debug("GStreamerStream for Camera index:{}",camera.index);
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder,m_link_handle,m_opt_action_handler);
      stream->setup();
      stream->start();
      m_camera_streams.push_back(stream);
      break;
    }
    case CameraType::RPI_CSI_LIBCAMERA: {
      m_console->debug("LibCamera index:{}", camera.index);
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder, m_link_handle,m_opt_action_handler);
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
  if(m_camera_streams.size()==1){
    m_camera_streams[0]->handle_change_bitrate_request(lb);
    return;
  }
  if(m_camera_streams.size()==2){
    const auto primary_perc=m_generic_settings->get_settings().dualcam_primary_video_allocated_bandwidth_perc;
    const int bitrate_primary_kbits=lb.recommended_encoder_bitrate_kbits*primary_perc/100;
    const int bitrate_secondary_kbits=lb.recommended_encoder_bitrate_kbits-bitrate_primary_kbits;
    openhd::ActionHandler::LinkBitrateInformation lb1{bitrate_primary_kbits};
    openhd::ActionHandler::LinkBitrateInformation lb2{bitrate_secondary_kbits};
    m_camera_streams[0]->handle_change_bitrate_request(lb1);
    m_camera_streams[1]->handle_change_bitrate_request(lb2);
    return ;
  }
  m_console->warn("openhd should always have either 1 or 2 cameras");
}

std::vector<openhd::Setting> OHDVideoAir::get_generic_settings() {
  std::vector<openhd::Setting> ret;
  // Camera related, but strongly interacts with the OS
  // This way one can switch between different OS configuration(s) that then provide access to different
  // vendor-specific camera(s) - hacky/dirty I know ;/
  if(m_platform.platform_type==PlatformType::RaspberryPi){
    assert(m_rpi_os_change_config_handler!= nullptr);
    auto c_rpi_os_camera_configuration=[this](std::string,int value){
      return m_rpi_os_change_config_handler->change_rpi_os_camera_configuration(value);
    };
    ret.push_back(openhd::Setting{"V_OS_CAM_CONFIG",openhd::IntSetting {openhd::rpi::os::cam_config_to_int(openhd::rpi::os::get_current_cam_config_from_file()),
                                                                        c_rpi_os_camera_configuration}});
  }
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
    ret.push_back(openhd::Setting{"V_SWITCH_CAM",openhd::IntSetting{m_generic_settings->get_settings().switch_primary_and_secondary,cb_switch_primary_and_secondary}});
  }
  if(n_cameras>1){
    auto cb=[this](std::string,int value){
      if(!is_valid_dualcam_primary_video_allocated_bandwidth(value))return false;
      m_generic_settings->unsafe_get_settings().dualcam_primary_video_allocated_bandwidth_perc=value;
      m_generic_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{"V_PRIMARY_PERC",openhd::IntSetting{m_generic_settings->get_settings().dualcam_primary_video_allocated_bandwidth_perc,cb}});
  }
  /*if(true){
    auto cb=[this](std::string,int value){
      if(!(value==1 || value==2))return false;
      m_generic_settings->unsafe_get_settings().n_cameras_to_wait_for=value;
      m_generic_settings->persist();
      // Do nothing, switch requires reboot
      return true;
    };
    ret.push_back(openhd::Setting{"V_N_CAMERAS",openhd::IntSetting{m_generic_settings->unsafe_get_settings().n_cameras_to_wait_for,cb}});
  }*/
  return ret;
}
