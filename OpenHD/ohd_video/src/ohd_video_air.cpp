//
// Created by consti10 on 03.05.22.
//
//#include <openhd-global-constants.hpp>
#include "ohd_video_air.h"

#include <utility>

#include "camera_discovery.h"
#include "gstreamerstream.h"
#include "openhd_config.h"
#include "gst_recording_demuxer.h"
#include "nalu/fragment_helper.h"

OHDVideoAir::OHDVideoAir(OHDPlatform platform1,std::vector<Camera> cameras,
                   std::shared_ptr<OHDLink> link)
    : m_platform(platform1),
      m_link_handle(std::move(link))
{
  m_console = openhd::log::create_or_get("v_air");
  assert(m_console);
  assert(!cameras.empty());
  m_console->debug("OHDVideo::OHDVideo()");
  m_primary_video_forwarder = std::make_unique<SocketHelper::UDPMultiForwarder>();
  m_secondary_video_forwarder = std::make_unique<SocketHelper::UDPMultiForwarder>();
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
    camera_holders.emplace_back(std::make_unique<CameraHolder>(camera));
  }
  startup_fix_common_issues(camera_holders);
  assert(camera_holders.size()<=MAX_N_CAMERAS);
  for (auto &camera: camera_holders) {
    configure(camera);
  }
    openhd::LinkActionHandler::instance().action_request_bitrate_change_register([this](openhd::LinkActionHandler::LinkBitrateInformation lb){
      this->handle_change_bitrate_request(lb);
    });
    auto cb_armed=[this](bool armed){
          this->update_arming_state(armed);
    };
    openhd::ArmingStateHelper::instance().register_listener("ohd_video_air",cb_armed);

  if(m_platform.platform_type==PlatformType::RaspberryPi){
    m_rpi_os_change_config_handler=std::make_unique<openhd::rpi::os::ConfigChangeHandler>(m_platform);
  }
  // On air, we start forwarding video (UDP) to all connected external device(s)
  openhd::ExternalDeviceManager::instance().register_listener([this](openhd::ExternalDevice external_device,bool connected){
    start_stop_forwarding_external_device(external_device,connected);
  });
  // In case any non-demuxed recordings exists (e.g. due to a openhd crash, unsafe shutdown,...)
  //GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
  m_console->debug( "OHDVideo::running");
}

OHDVideoAir::~OHDVideoAir() {
    openhd::ArmingStateHelper::instance().unregister_listener("ohd_video_air");
    openhd::LinkActionHandler::instance().action_request_bitrate_change_register(nullptr);
    // Stop all the camera stream(s)
    m_camera_streams.resize(0);
}

void OHDVideoAir::configure(const std::shared_ptr<CameraHolder>& camera_holder) {
  const auto camera=camera_holder->get_camera();
  m_console->debug("Configuring camera:"+camera_type_to_string(camera.type));
  auto frame_cb=[this](int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame){
      this->on_video_data(stream_index,fragmented_video_frame);
  };
  // R.N we use gstreamer for pretty much everything
  // But this might change in the future
  switch (camera.type) {
    case CameraType::RPI_CSI_VEYE_V4l2:
    case CameraType::RPI_CSI_MMAL:
    case CameraType::JETSON_CSI:
    case CameraType::IP:
    case CameraType::ROCKCHIP_CSI:
    case CameraType::ALLWINNER_CSI:
    case CameraType::UVC:
    case CameraType::ROCKCHIP_HDMI:
    case CameraType::CUSTOM_UNMANAGED_CAMERA:
    case CameraType::RPI_CSI_LIBCAMERA:
    case CameraType::DUMMY_SW: {
      m_console->debug("GStreamerStream for Camera index:{}",camera.index);
      auto stream = std::make_shared<GStreamerStream>(m_platform.platform_type, camera_holder,frame_cb);
      stream->start_looping();
      m_camera_streams.push_back(stream);
      break;
    }
    default: {
      m_console->error("Unknown camera type, skipping");
    }
  }
}

std::vector<std::shared_ptr<openhd::ISettingsComponent>>
OHDVideoAir::get_all_camera_settings() {
  std::vector<std::shared_ptr<openhd::ISettingsComponent>> ret;
  ret.reserve(m_camera_streams.size());
  for(auto& stream: m_camera_streams){
    ret.push_back(stream->m_camera_holder);
  }
  return ret;
}

void OHDVideoAir::handle_change_bitrate_request(openhd::LinkActionHandler::LinkBitrateInformation lb) {
  if(m_camera_streams.size()==1){
    m_camera_streams[0]->handle_change_bitrate_request(lb);
    return;
  }
  if(m_camera_streams.size()==2){
    // Just split the available bitrate between primary and secondary cam, according to the user's preferences
    const auto primary_perc=m_generic_settings->get_settings().dualcam_primary_video_allocated_bandwidth_perc;
    const int bitrate_primary_kbits=lb.recommended_encoder_bitrate_kbits*primary_perc/100;
    const int bitrate_secondary_kbits=lb.recommended_encoder_bitrate_kbits-bitrate_primary_kbits;
    openhd::LinkActionHandler::LinkBitrateInformation lb1{bitrate_primary_kbits};
    openhd::LinkActionHandler::LinkBitrateInformation lb2{bitrate_secondary_kbits};
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
  // Allows changin the camera(s) openhd waits for at boot via mavlink. Requires reboot
  if(true){
    auto cb=[this](std::string,int value){
      if(!(value==1 || value==2))return false;
      m_generic_settings->unsafe_get_settings().n_cameras_to_wait_for=value;
      m_generic_settings->persist();
      // change requires reboot
      return true;
    };
    ret.push_back(openhd::Setting{"V_N_CAMERAS",openhd::IntSetting{m_generic_settings->get_settings().n_cameras_to_wait_for,cb}});
  }
  // Only show dual-cam settings if dual-cam is actually used
  const auto n_cameras=static_cast<int>(m_camera_streams.size());
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
  return ret;
}

static const auto CUSTOM_UNMANAGED_CAMERA_SERVICE_NAME="custom_unmanaged_camera";

std::vector<Camera> OHDVideoAir::discover_cameras(const OHDPlatform& platform) {
  auto m_console=openhd::log::get_default();
  const auto config=openhd::load_config();
  // Default camera autodetect - wait for camera(s) to become available, but to not infinitely blcok the boot
  // process, if not enough camera(s) have been found after a given timespan, use dummy camera(s) for them
  if(config.CAMERA_ENABLE_AUTODETECT){
    int n_wanted_cameras=AirCameraGenericSettingsHolder{}.get_settings().n_cameras_to_wait_for;
    if(n_wanted_cameras>2 || n_wanted_cameras<=0){
      m_console->warn("Invalid n cameras {}",n_wanted_cameras);
      n_wanted_cameras=1;
    }
    m_console->debug("Waiting for {} cameras.",n_wanted_cameras);
    std::vector<Camera> cameras{};
    // Default - works well with csi and usb cameras
    // Issue on rpi: The openhd service is often started before ? (most likely the OS needs to do some internal setup stuff)
    // and then the cameras discovery step is run before the camera is available, and therefore not found. Block up to
    // X seconds here, to give the OS time until the camera is available, only then continue with the dummy camera
    // Since the jetson is also an embedded platform, just like the rpi, I am doing it for it too, even though I never
    // checked if that's actually an issue there
    cameras = DCameras::discover(platform);
    // Always wait
    if(true) {
      const auto begin = std::chrono::steady_clock::now();
      while (std::chrono::steady_clock::now() - begin <std::chrono::seconds(10)) {
        if (cameras.size()>=n_wanted_cameras) {
          m_console->debug("Done waiting for camera(s),found {}",cameras.size());
          // break as soon as we have at least enough cameras
          break;
        }
        const int sleep_time_seconds=3;
        openhd::log::get_default()->debug("Re-running camera discovery step, until camera is found/timeout. Sleep for {} seconds",sleep_time_seconds);
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time_seconds));
        cameras = DCameras::discover(platform);
      }
    }
    m_console->debug("Done waiting for camera(s), wanted: {}, actual:{}",n_wanted_cameras,cameras.size());
    for(int i=(int)cameras.size();i<n_wanted_cameras;i++){
      m_console->warn("Adding dummy camera {}",i);
      cameras.emplace_back(createDummyCamera(i));
    }
    return cameras;
  }
  m_console->info("Camera autodetect off");
  // Autodetect off
  std::vector<Camera> cameras{};
  std::vector<CameraType> camera_types;
  bool start_custom_unamanged_camera_service= false;
  // One cam is always required
  for(int i=0;i<config.CAMERA_N_CAMERAS;i++){
    const auto cam_type_str=i==0 ? config.CAMERA_CAMERA0_TYPE : config.CAMERA_CAMERA1_TYPE;
    const auto cam_type= camera_type_from_string(cam_type_str);
    camera_types.push_back(cam_type);
    // check if we need to start the "whatever the user wants" service
    if(cam_type==CameraType::CUSTOM_UNMANAGED_CAMERA){
      start_custom_unamanged_camera_service= true;
    }
  }
  // In general, for cameras that support autodetection, if the user manually specified he wants one of them,
  // we wait for at least one to become available
  for(int i=0;i<camera_types.size();i++){
    const auto cam_type=camera_types.at(i);
    if(cam_type==CameraType::DUMMY_SW){
      cameras.emplace_back(createDummyCamera(i));
    }else if(cam_type==CameraType::CUSTOM_UNMANAGED_CAMERA){
      cameras.emplace_back(createCustomUnmanagedCamera(i));
    }else if(cam_type==CameraType::RPI_CSI_MMAL){
      auto mmal_cameras=DCameras::detect_raspberrypi_broadcom_csi(m_console);
      // Wait for camera to become available
      while (mmal_cameras.empty()){
        m_console->debug("Waiting for mmal camera");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mmal_cameras=DCameras::detect_raspberrypi_broadcom_csi(m_console);
      }
      cameras.emplace_back(mmal_cameras.at(0));
    }else if(cam_type==CameraType::RPI_CSI_LIBCAMERA){
      auto libcamera_cameras=DCameras::detect_raspberrypi_libcamera_csi(m_console);
      // Wait for camera to become available
      while (libcamera_cameras.empty()){
        m_console->debug("Waiting for libcamera camera");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        libcamera_cameras=DCameras::detect_raspberrypi_libcamera_csi(m_console);
      }
      cameras.emplace_back(libcamera_cameras.at(0));
    }else if(cam_type==CameraType::RPI_CSI_VEYE_V4l2){
      auto veye_cameras=DCameras::detect_rapsberrypi_veye_v4l2_dirty(m_console);
      // Wait for camera to become available
      while (veye_cameras.empty()){
        m_console->debug("Waiting for veye camera");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        veye_cameras=DCameras::detect_rapsberrypi_veye_v4l2_dirty(m_console);
      }
      cameras.emplace_back(veye_cameras.at(0));
    }else if(cam_type==CameraType::UVC){
      auto usb_cameras=DCameras::detect_usb_cameras(platform,m_console);
      // Wait for camera to become available
      while (usb_cameras.empty()){
        m_console->debug("Waiting for usb camera");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        usb_cameras=DCameras::detect_usb_cameras(platform,m_console);
      }
      cameras.emplace_back(usb_cameras.at(0));
    }
    else{
      m_console->warn("Unsupported manual camera type {}", camera_type_to_string(cam_type));
    }
  }
  if(cameras.empty()){
    openhd::log::get_default()->warn("autodetect off but no cameras. Fix file and restart openhd");
    OHDUtil::keep_alive_until_sigterm();
  }
  if(start_custom_unamanged_camera_service){
    OHDUtil::run_command("systemctl",{"start",CUSTOM_UNMANAGED_CAMERA_SERVICE_NAME});
  }
  return cameras;
}

void OHDVideoAir::start_stop_forwarding_external_device(openhd::ExternalDevice external_device, bool connected) {
    const std::string client_addr=external_device.external_device_ip;
    if(connected){
        m_primary_video_forwarder->addForwarder(client_addr,5600);
        m_secondary_video_forwarder->addForwarder(client_addr,5601);
        m_has_localhost_forwarding_enabled= true;
    }else{
        m_has_localhost_forwarding_enabled=false;
        m_primary_video_forwarder->removeForwarder(client_addr,5600);
        m_secondary_video_forwarder->removeForwarder(client_addr,5601);
    }
}

void OHDVideoAir::on_video_data(int stream_index, const openhd::FragmentedVideoFrame &fragmented_video_frame) {
    //m_console->debug("Got data {} {}",stream_index,fragmented_video_frame.frame_fragments.size());
    if(!(stream_index==0 || stream_index==1)){
        m_console->debug("Invalid stream index: {}",stream_index);
        return;
    }
    if(m_link_handle){
        m_link_handle->transmit_video_data(stream_index,fragmented_video_frame);
    }
    if(m_has_localhost_forwarding_enabled){
        //m_console->debug("Forwarding {} {}",stream_index,fragmented_video_frame.frame_fragments.size());
        auto& forwarder=stream_index==0 ? m_primary_video_forwarder : m_secondary_video_forwarder;
        for(auto& fragment:fragmented_video_frame.frame_fragments){
            forwarder->forwardPacketViaUDP(fragment->data(),fragment->size());
        }
        if(fragmented_video_frame.dirty_frame){
            auto fragments=make_fragments(fragmented_video_frame.dirty_frame->data(),fragmented_video_frame.dirty_frame->size());
            for(auto& fragment:fragments){
                forwarder->forwardPacketViaUDP(fragment->data(),fragment->size());
            }
        }
    }
}

void OHDVideoAir::update_arming_state(bool armed) {
    for(auto& camera:m_camera_streams){
        camera->handle_update_arming_state(armed);
    }
}
