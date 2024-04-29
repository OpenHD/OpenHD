//
// Created by consti10 on 03.05.22.
//
#include "ohd_video_air.h"

#include <utility>

#include "camera_discovery.h"
#include "gstaudiostream.h"
#include "gstreamerstream.h"
#include "nalu/fragment_helper.h"
#include "openhd_config.h"
#include "openhd_reboot_util.h"

OHDVideoAir::OHDVideoAir(std::vector<XCamera> cameras,
                         std::shared_ptr<OHDLink> link)
    : m_link_handle(std::move(link)) {
  m_console = openhd::log::create_or_get("v_air");
  assert(m_console);
  assert(!cameras.empty());
  m_console->debug("OHDVideo::OHDVideo()");
  m_primary_video_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  m_secondary_video_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  m_audio_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  if (cameras.size() > MAX_N_CAMERAS) {
    m_console->warn("More than {} cameras, dropping cameras", MAX_N_CAMERAS);
    cameras.resize(MAX_N_CAMERAS);
  }
  m_generic_settings = std::make_unique<AirCameraGenericSettingsHolder>();
  if (OHDPlatform::instance().is_x20()) {
    m_generic_settings->x20_only_discover_and_save_camera_type();
  }
  if (m_generic_settings->get_settings().switch_primary_and_secondary &&
      cameras.size() == 2) {
    // swap cam 1 and cam 2 (primary and secondary) - aka if they are detected
    // in the wrong order
    auto cam1 = cameras.at(1);
    auto cam2 = cameras.at(0);
    cam1.index = 0;
    cam2.index = 1;
    cameras.resize(0);
    cameras.push_back(cam1);
    cameras.push_back(cam2);
  }
  std::vector<std::shared_ptr<CameraHolder>> camera_holders;
  for (const auto& camera : cameras) {
    camera_holders.emplace_back(std::make_unique<CameraHolder>(camera));
  }
  assert(camera_holders.size() <= MAX_N_CAMERAS);
  for (auto& camera : camera_holders) {
    configure(camera);
  }
  if (m_generic_settings->get_settings().enable_audio != OPENHD_AUDIO_DISABLE) {
    m_audio_stream = std::make_unique<GstAudioStream>();
    auto audio_cb = [this](const openhd::AudioPacket& audioPacket) {
      on_audio_data(audioPacket);
    };
    m_audio_stream->set_link_cb(audio_cb);
    if (m_generic_settings->get_settings().enable_audio == OPENHD_AUDIO_TEST) {
      m_audio_stream->openhd_enable_audio_test = true;
    } else {
      m_audio_stream->openhd_enable_audio_test = false;
    }
    m_audio_stream->start_looping();
  }
  openhd::LinkActionHandler::instance().action_request_bitrate_change_register(
      [this](openhd::LinkActionHandler::LinkBitrateInformation lb) {
        this->handle_change_bitrate_request(lb);
      });
  auto cb_armed = [this](bool armed) { this->update_arming_state(armed); };
  openhd::ArmingStateHelper::instance().register_listener("ohd_video_air",
                                                          cb_armed);
  // On air, we start forwarding video (UDP) to all connected external device(s)
  openhd::ExternalDeviceManager::instance().register_listener(
      [this](openhd::ExternalDevice external_device, bool connected) {
        start_stop_forwarding_external_device(external_device, connected);
      });
  // In case any non-demuxed recordings exists (e.g. due to a openhd crash,
  // unsafe shutdown,...)
  // GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
  m_console->debug("OHDVideo::running");
}

OHDVideoAir::~OHDVideoAir() {
  openhd::ArmingStateHelper::instance().unregister_listener("ohd_video_air");
  openhd::LinkActionHandler::instance().action_request_bitrate_change_register(
      nullptr);
  // Stop all the camera stream(s)
  m_camera_streams.resize(0);
  // stop audio if running
  m_audio_stream = nullptr;
}

void OHDVideoAir::configure(
    const std::shared_ptr<CameraHolder>& camera_holder) {
  const auto camera = camera_holder->get_camera();
  auto frame_cb =
      [this](int stream_index,
             const openhd::FragmentedVideoFrame& fragmented_video_frame) {
        this->on_video_data(stream_index, fragmented_video_frame);
      };
  // R.N we use gstreamer for pretty much everything
  // But this might change in the future
  m_console->debug("GStreamerStream for Camera index:{}", camera.index);
  auto stream = std::make_shared<GStreamerStream>(camera_holder, frame_cb);
  stream->start_looping();
  m_camera_streams.push_back(stream);
}

std::array<std::vector<openhd::Setting>, 2>
OHDVideoAir::get_all_camera_settings() {
  std::array<std::vector<openhd::Setting>, 2> ret{};
  // On all platforms other than X20 the user (needs) to select the camera
  // type. On X20, we expose the param, but only for read-only / debugging
  for (int i = 0; i < 2; i++) {
    // Changing the cam type is special - it requires a restart of openhd
    // (often also an OS reboot)
    auto cb1 = [this](std::string, int value) {
      if (OHDPlatform::instance().is_x20()) return false;
      return x_set_camera_type(true, value);
    };
    auto cb2 = [this](std::string, int value) {
      if (OHDPlatform::instance().is_x20()) return false;
      return x_set_camera_type(false, value);
    };
    if (i == 0) {
      ret[0].push_back(openhd::Setting{
          "CAMERA_TYPE",
          openhd::IntSetting{
              m_generic_settings->get_settings().primary_camera_type, cb1}});
    } else {
      ret[1].push_back(openhd::Setting{
          "CAMERA_TYPE",
          openhd::IntSetting{
              m_generic_settings->get_settings().secondary_camera_type, cb2}});
    }
    // Then add the generic camera settings (there might be none)
    if (i < m_camera_streams.size()) {
      auto settings = m_camera_streams[i]->m_camera_holder->get_all_settings();
      for (auto& setting : settings) {
        ret[i].push_back(setting);
      }
    }
  }
  return ret;
}

std::vector<openhd::Setting> OHDVideoAir::get_generic_settings() {
  std::vector<openhd::Setting> ret;
  // Only show dual-cam settings if dual-cam is actually used
  const auto n_cameras = static_cast<int>(m_camera_streams.size());
  if (n_cameras > 1) {
    auto cb_switch_primary_and_secondary = [this](std::string, int value) {
      if (!openhd::validate_yes_or_no(value)) return false;
      m_generic_settings->unsafe_get_settings().switch_primary_and_secondary =
          value;
      m_generic_settings->persist();
      // Do nothing, switch requires reboot
      return true;
    };
    ret.push_back(openhd::Setting{
        "V_SWITCH_CAM",
        openhd::IntSetting{
            m_generic_settings->get_settings().switch_primary_and_secondary,
            cb_switch_primary_and_secondary}});
  }
  if (n_cameras > 1) {
    auto cb = [this](std::string, int value) {
      if (!is_valid_dualcam_primary_video_allocated_bandwidth(value))
        return false;
      m_generic_settings->unsafe_get_settings()
          .dualcam_primary_video_allocated_bandwidth_perc = value;
      m_generic_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{
        "V_PRIMARY_PERC",
        openhd::IntSetting{m_generic_settings->get_settings()
                               .dualcam_primary_video_allocated_bandwidth_perc,
                           cb}});
  }
  if (!OHDPlatform::instance().is_x20()) {
    auto cb_audio = [this](std::string, int value) {
      m_generic_settings->unsafe_get_settings().enable_audio = value;
      m_generic_settings->persist();
      openhd::TerminateHelper::instance().terminate_after(
          "Audio", std::chrono::seconds(1));
      return true;
    };
    ret.push_back(openhd::Setting{
        "AUDIO_ENABLE",
        openhd::IntSetting{m_generic_settings->get_settings().enable_audio,
                           cb_audio}});
  }
  return ret;
}

void OHDVideoAir::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  if (m_camera_streams.size() == 1) {
    m_camera_streams[0]->handle_change_bitrate_request(lb);
    return;
  }
  if (m_camera_streams.size() == 2) {
    // Just split the available bitrate between primary and secondary cam,
    // according to the user's preferences
    const auto primary_perc =
        m_generic_settings->get_settings()
            .dualcam_primary_video_allocated_bandwidth_perc;
    const int bitrate_primary_kbits =
        lb.recommended_encoder_bitrate_kbits * primary_perc / 100;
    const int bitrate_secondary_kbits =
        lb.recommended_encoder_bitrate_kbits - bitrate_primary_kbits;
    openhd::LinkActionHandler::LinkBitrateInformation lb1{
        bitrate_primary_kbits};
    openhd::LinkActionHandler::LinkBitrateInformation lb2{
        bitrate_secondary_kbits};
    m_camera_streams[0]->handle_change_bitrate_request(lb1);
    m_camera_streams[1]->handle_change_bitrate_request(lb2);
    return;
  }
  m_console->warn("openhd should always have either 1 or 2 cameras");
}

void OHDVideoAir::start_stop_forwarding_external_device(
    openhd::ExternalDevice external_device, bool connected) {
  const std::string client_addr = external_device.external_device_ip;
  if (connected) {
    m_primary_video_forwarder->addForwarder(client_addr, 5600);
    m_secondary_video_forwarder->addForwarder(client_addr, 5601);
    m_audio_forwarder->addForwarder(client_addr, 5610);
    m_has_localhost_forwarding_enabled = true;
  } else {
    m_has_localhost_forwarding_enabled = false;
    m_primary_video_forwarder->removeForwarder(client_addr, 5600);
    m_secondary_video_forwarder->removeForwarder(client_addr, 5601);
    m_audio_forwarder->removeForwarder(client_addr, 5610);
  }
}

void OHDVideoAir::on_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  // m_console->debug("Got data {}
  // {}",stream_index,fragmented_video_frame.rtp_fragments.size());
  if (!(stream_index == 0 || stream_index == 1)) {
    m_console->debug("Invalid stream index: {}", stream_index);
    return;
  }
  if (m_link_handle) {
    m_link_handle->transmit_video_data(stream_index, fragmented_video_frame);
  }
  if (m_has_localhost_forwarding_enabled) {
    // m_console->debug("Forwarding {}
    // {}",stream_index,fragmented_video_frame.rtp_fragments.size());
    auto& forwarder = stream_index == 0 ? m_primary_video_forwarder
                                        : m_secondary_video_forwarder;
    for (auto& fragment : fragmented_video_frame.rtp_fragments) {
      forwarder->forwardPacketViaUDP(fragment->data(), fragment->size());
    }
    if (fragmented_video_frame.dirty_frame) {
      auto fragments =
          make_fragments(fragmented_video_frame.dirty_frame->data(),
                         fragmented_video_frame.dirty_frame->size());
      for (auto& fragment : fragments) {
        forwarder->forwardPacketViaUDP(fragment->data(), fragment->size());
      }
    }
  }
}

void OHDVideoAir::on_audio_data(const openhd::AudioPacket& audio_packet) {
  if (m_link_handle) {
    m_link_handle->transmit_audio_data(audio_packet);
  }
  if (m_has_localhost_forwarding_enabled) {
    m_audio_forwarder->forwardPacketViaUDP(audio_packet.data->data(),
                                           audio_packet.data->size());
  }
}

void OHDVideoAir::update_arming_state(bool armed) {
  for (auto& camera : m_camera_streams) {
    camera->handle_update_arming_state(armed);
  }
}

#ifdef ENABLE_USB_CAMERAS
static std::vector<int> x_discover_usb_cameras(int num_usb_cameras) {
  const auto platform = OHDPlatform::instance();
  auto console = openhd::log::get_default();
  const auto discovery_begin = std::chrono::steady_clock::now();
  console->debug("Waiting for usb camera(s)");
  std::vector<DCameras::DiscoveredUSBCamera> usb_cameras;
  while (true) {
    usb_cameras = DCameras::detect_usb_cameras(console, false);
    if (usb_cameras.size() >= num_usb_cameras) {
      break;
    }
    if (std::chrono::steady_clock::now() - discovery_begin >
        std::chrono::seconds(10)) {
      console->warn("Cannot find usb camera(s)");
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::vector<int> ret;
  for (int i = 0; i < num_usb_cameras; i++) {
    if (i < usb_cameras.size()) {
      ret.push_back(usb_cameras[i].v4l2_device_number);
    } else {
      // We didn't find the usb cam - guess it
      const int guess_bus_name = i == 0 ? 0 : 1;
      ret.push_back(guess_bus_name);
    }
  }
  return ret;
}

static int get_num_usb_cameras(const int primary_camera_type,
                               const int secondary_camera_type) {
  int num_usb_cameras = 0;
  if (is_usb_camera(primary_camera_type)) {
    num_usb_cameras++;
  }
  if (is_usb_camera(secondary_camera_type)) {
    num_usb_cameras++;
  }
  return num_usb_cameras;
}
#endif
std::vector<XCamera> OHDVideoAir::discover_cameras() {
  auto platform = OHDPlatform::instance();
  auto global_settings_holder =
      std::make_unique<AirCameraGenericSettingsHolder>();
  auto global_settings = global_settings_holder->get_settings();
  auto console = openhd::log::get_default();

  const int num_active_cameras =
      global_settings.secondary_camera_type == X_CAM_TYPE_DISABLED ? 1 : 2;
  std::vector<int> usb_cam_bus_names;
#ifdef ENABLE_USB_CAMERAS
  const int num_usb_cameras =
      get_num_usb_cameras(global_settings.primary_camera_type,
                          global_settings.secondary_camera_type);
  if (num_usb_cameras > 0) {
    // ONLY usb cameras we need to discover
    usb_cam_bus_names = x_discover_usb_cameras(num_usb_cameras);
  }
#endif
  std::vector<XCamera> ret;
  int usb_cameras_offset = 0;
  for (int i = 0; i < num_active_cameras; i++) {
    auto cam_type = i == 0 ? global_settings.primary_camera_type
                           : global_settings.secondary_camera_type;
    if (is_usb_camera(cam_type)) {
      ret.push_back(
          XCamera{cam_type, i, usb_cam_bus_names[usb_cameras_offset]});
      usb_cameras_offset++;
    } else {
      ret.push_back(XCamera{cam_type, i, 0});
    }
  }
  return ret;
}

bool OHDVideoAir::x_set_camera_type(bool primary, int cam_type) {
  // Validation depends on primary / secondary -we are quite lazy here
  if (primary) {
    if (!is_valid_primary_cam_type(cam_type)) return false;
  } else {
    if (!is_valid_secondary_cam_type(cam_type)) return false;
  }
  if (primary) {
    m_generic_settings->unsafe_get_settings().primary_camera_type = cam_type;
    openhd::LinkActionHandler::instance().set_cam_info_type(0, cam_type);
  } else {
    m_generic_settings->unsafe_get_settings().secondary_camera_type = cam_type;
    openhd::LinkActionHandler::instance().set_cam_info_type(1, cam_type);
  }
  bool reboot_required = false;
  if (primary) {
    if (OHDPlatform::instance().is_rpi() && is_rpi_csi_camera(cam_type) ||
        OHDPlatform::instance().is_rock() && is_rock_csi_camera(cam_type)) {
      openhd::log::get_default()->warn(
          "Calling image cam helper for cam type {}({})", cam_type,
          x_cam_type_to_string(cam_type));
      auto res = OHDUtil::run_command_out(
          fmt::format("bash /usr/local/bin/ohd_camera_setup.sh {}", cam_type),
          {});
      openhd::log::get_default()->debug("script returned:[{}]",
                                        res.value_or("ERROR"));
      reboot_required = true;
    }
  }
  m_generic_settings->persist(false);
  if (reboot_required) {
    openhd::reboot::handle_power_command_async(std::chrono::seconds(1), false);
  } else {
    // Restarting openhd is enough
    openhd::TerminateHelper::instance().terminate_after(
        "CameraType", std::chrono::seconds(1));
  }
  return true;
}
