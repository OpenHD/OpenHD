//
// Created by consti10 on 19.09.23.
//
#include "camera_holder.h"
#include "include_json.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM( CameraType, {
    {CameraType::UNKNOWN, nullptr},
    {CameraType::DUMMY_SW, "DUMMY_SW"},
    {CameraType::RPI_CSI_MMAL, "RPI_CSI_MMAL"},
    {CameraType::RPI_CSI_VEYE_V4l2, "RPI_CSI_VEYE_V4l2"},
    {CameraType::JETSON_CSI, "JETSON_CSI"},
    {CameraType::ROCKCHIP_CSI, "ROCKCHIP_CSI"},
    {CameraType::ALLWINNER_CSI, "ALLWINNER_CSI"},
    {CameraType::UVC, "UVC"},
    {CameraType::UVC_H264, "UVC_H264"},
    {CameraType::IP, "IP"},
    {CameraType::RPI_CSI_LIBCAMERA, "RPI_CSI_LIBCAMERA"},
    {CameraType::ROCKCHIP_HDMI, "ROCKCHIP_HDMI"},
    {CameraType::CUSTOM_UNMANAGED_CAMERA, "CUSTOM_UNMANAGED_CAMERA"}
});

NLOHMANN_JSON_SERIALIZE_ENUM( VideoCodec, {
    {VideoCodec::H264, "h264"},
    {VideoCodec::H265, "h265"},
    {VideoCodec::MJPEG, "mjpeg"},
});

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoFormat,videoCodec,width,height,framerate)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EndpointFormat,format,width,height,fps)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IPCameraSettings,location,extraparam_1)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraEndpointV4l2, v4l2_device_node,bus,formats_h264,formats_h265,formats_mjpeg,formats_raw)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,sensor_name,bus,index,rpi_csi_mmal_is_csi_to_hdmi, v4l2_endpoints)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings, enable_streaming,
                                   streamed_video_format, h26x_bitrate_kbits,
                                   h26x_keyframe_interval, h26x_intra_refresh_type, mjpeg_quality_percent, ip_cam_url, air_recording,
                                   camera_rotation_degree, horizontal_flip, vertical_flip,
                                   awb_mode, exposure_mode, brightness_percentage, rpi_rpicamsrc_iso, rpi_rpicamsrc_metering_mode,
                                    // rpi libcamera specific IQ params begin
                                   rpi_libcamera_sharpness_as_int,rpi_libcamera_contrast_as_int,rpi_libcamera_saturation_as_int,rpi_libcamera_ev_value,
                                   rpi_libcamera_denoise_index, rpi_libcamera_awb_index, rpi_libcamera_metering_index, rpi_libcamera_exposure_index,
                                   rpi_libcamera_shutter_microseconds,
                                    // rpi libcamera specific IQ params end
                                   force_sw_encode,
                                   enable_ultra_secure_encryption)

std::optional<CameraSettings> CameraHolder::impl_deserialize(const std::string &file_as_string) const {
    return openhd_json_parse<CameraSettings>(file_as_string);
}

std::string CameraHolder::imp_serialize(const CameraSettings &data) const {
    const nlohmann::json tmp=data;
    return tmp.dump(4);
}

std::vector<openhd::Setting> CameraHolder::get_all_settings() {
  auto c_enable_streaming=[this](std::string,int value) {
    return set_enable_streaming(value);
  };
  auto c_codec=[this](std::string, int value) {
    return set_video_codec(value);
  };
  auto c_recording=[this](std::string,int value) {
    return set_air_recording(value);
  };
  std::vector<openhd::Setting> ret={
      openhd::Setting{"V_E_STREAMING",openhd::IntSetting{get_settings().enable_streaming,c_enable_streaming}},
      openhd::Setting{"VIDEO_CODEC",openhd::IntSetting{video_codec_to_int(get_settings().streamed_video_format.videoCodec), c_codec}},
      openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{get_settings().air_recording,c_recording}},
      // for debugging
      openhd::create_read_only_string("V_CAM_TYPE",camera_type_to_string(m_camera.type)),
      openhd::create_read_only_string("V_CAM_NAME",m_camera.name)
  };
  if(m_camera.sensor_name!="unknown"){
    ret.emplace_back(openhd::create_read_only_string("V_CAM_SENSOR",m_camera.sensor_name));
  }
  if(m_camera.type==CameraType::IP){
    auto cb_ip_cam_url=[this](std::string,std::string value){
      return set_ip_cam_url(value);
    };
    ret.push_back(openhd::Setting{"V_IP_CAM_URL",openhd::StringSetting {get_settings().ip_cam_url,cb_ip_cam_url}});
  }
  if(m_camera.supports_force_sw_encode()){
    auto cb=[this](std::string,int value){
      if(!openhd::validate_yes_or_no(value))return false;
      unsafe_get_settings().force_sw_encode = value;
      persist();
      return true;
    };
    ret.push_back(openhd::Setting{"V_FORCE_SW_ENC",openhd::IntSetting {get_settings().force_sw_encode,cb}});
  }
  if(m_camera.supports_bitrate()){
    // NOTE: OpenHD stores the bitrate in kbit/s, but for now we use MBit/s for the setting
    // (Since that is something a normal user can make more sense of)
    // and just multiply the value in the callback
    auto c_bitrate=[this](std::string,int value) {
      return set_video_bitrate(value);
    };
    auto c_mjpeg_quality_percent=[this](std::string,int value) {
      return set_mjpeg_quality_percent(value);
    };
    ret.push_back(openhd::Setting{"V_BITRATE_MBITS",openhd::IntSetting{static_cast<int>(get_settings().h26x_bitrate_kbits / 1000),c_bitrate}});
    if(get_settings().streamed_video_format.videoCodec==VideoCodec::MJPEG){
      ret.push_back(openhd::Setting{"V_MJPEG_QUALITY",openhd::IntSetting{get_settings().mjpeg_quality_percent,c_mjpeg_quality_percent}});
    }
  }
  if(m_camera.supports_changing_format()){
    auto c_width_height_framerate=[this](std::string,std::string value){
      auto tmp_opt=openhd::parse_video_format(value);
      if(tmp_opt.has_value()){
        const auto& tmp=tmp_opt.value();
        return set_video_width_height_framerate(tmp.width_px,tmp.height_px,tmp.framerate);
      }
      return false;
    };
    // Width, height and FPS are done together now (V_FORMAT)
    const auto format_string=openhd::video_format_from_int_values(get_settings().streamed_video_format.width,
                                                                  get_settings().streamed_video_format.height,
                                                                  get_settings().streamed_video_format.framerate);
    ret.push_back(openhd::Setting{"V_FORMAT",openhd::StringSetting{format_string,c_width_height_framerate}});
  }
  if(m_camera.supports_keyframe_interval()){
    auto c_keyframe_interval=[this](std::string,int value) {
      return set_keyframe_interval(value);
    };
    ret.push_back(openhd::Setting{"V_KEYFRAME_I",openhd::IntSetting{get_settings().h26x_keyframe_interval,c_keyframe_interval}});
  }
  if(m_camera.supports_awb()){
    auto cb=[this](std::string,int value) {
      return set_camera_awb(value);
    };
    ret.push_back(openhd::Setting{"V_AWB_MODE",openhd::IntSetting{get_settings().awb_mode,cb}});
  }
  if(m_camera.supports_exp()){
    auto cb=[this](std::string,int value) {
      return set_camera_exposure(value);
    };
    ret.push_back(openhd::Setting{"V_EXP_MODE",openhd::IntSetting{get_settings().exposure_mode,cb}});
  }
  if(m_camera.supports_rotation()){
    auto c_rotation=[this](std::string,int value) {
      return set_camera_rotation(value);
    };
    ret.push_back(openhd::Setting{"V_CAM_ROT_DEG",openhd::IntSetting{get_settings().camera_rotation_degree,c_rotation}});
  }
  if(m_camera.supports_hflip_vflip()){
    auto c_horizontal_flip=[this](std::string,int value) {
      return set_horizontal_flip(value);
    };
    auto c_vertical_flip=[this](std::string,int value) {
      return set_vertical_flip(value);
    };
    ret.push_back(openhd::Setting{"V_VERT_FLIP",openhd::IntSetting{get_settings().vertical_flip,c_vertical_flip}});
    ret.push_back(openhd::Setting{"V_HORIZ_FLIP",openhd::IntSetting{get_settings().horizontal_flip,c_horizontal_flip}});
  }
  if(true){ // Always show intra, on libcamera without sw encode it unfortunately is 'just not mapped' and ignored.
    auto c_intra_refresh_type=[this](std::string,int value) {
      return set_intra_refresh_type(value);
    };
    ret.push_back(openhd::Setting{"V_INTRA_REFRESH",openhd::IntSetting{get_settings().h26x_intra_refresh_type,c_intra_refresh_type}});
  }
  if(m_camera.supports_brightness()){
    auto c_brightness=[this](std::string,int value) {
      return set_brightness(value);
    };
    ret.push_back(openhd::Setting{"V_BRIGHTNESS",openhd::IntSetting{get_settings().brightness_percentage,c_brightness}});
  }
  if(m_camera.supports_sharpness()){
    // TODO
  }
  if(m_camera.supports_iso()){
    auto c_iso=[this](std::string,int value) {
      return set_rpi_rpicamsrc_iso(value);
    };
    ret.push_back(openhd::Setting{"V_ISO",openhd::IntSetting{get_settings().rpi_rpicamsrc_iso,c_iso}});
  }
  if(m_camera.supports_rpi_rpicamsrc_metering_mode()){
    auto cb=[this](std::string,int value) {
      return set_rpi_rpicamsrc_metering_mode(value);
    };
    ret.push_back(openhd::Setting{"V_METERING_MODE",openhd::IntSetting{get_settings().rpi_rpicamsrc_metering_mode,cb}});
  }
  // These are rpi libcamera specific image quality settings
  if(m_camera.type==CameraType::RPI_CSI_LIBCAMERA){
    auto cb_sharpness=[this](std::string,int value) {
      return set_rpi_libcamera_sharpness_as_int(value);
    };
    ret.push_back(openhd::Setting{"SHARPNESS_LC",openhd::IntSetting{get_settings().rpi_libcamera_sharpness_as_int,cb_sharpness}});
    auto cb_contrast=[this](std::string,int value) {
      return set_rpi_libcamera_contrast_as_int(value);
    };
    ret.push_back(openhd::Setting{"CONTRAST_LC",openhd::IntSetting{get_settings().rpi_libcamera_contrast_as_int,cb_contrast}});
    auto cb_saturation=[this](std::string,int value) {
      return set_rpi_libcamera_saturation_as_int(value);
    };
    ret.push_back(openhd::Setting{"SATURATION_LC",openhd::IntSetting{get_settings().rpi_libcamera_saturation_as_int,cb_saturation}});
    auto cb_ev=[this](std::string,int value) {
      return set_rpi_libcamera_ev_value(value);
    };
    ret.push_back(openhd::Setting{"EXPOSURE_LC",openhd::IntSetting{get_settings().rpi_libcamera_ev_value,cb_ev}});

    auto cb_denoise=[this](std::string,int value) {
      return set_rpi_libcamera_denoise_index(value);
    };
    ret.push_back(openhd::Setting{"DENOISE_INDEX_LC",openhd::IntSetting{get_settings().rpi_libcamera_denoise_index,cb_denoise}});
    auto cb_awb=[this](std::string,int value) {
      return set_rpi_libcamera_awb_index(value);
    };
    ret.push_back(openhd::Setting{"AWB_MODE_LC",openhd::IntSetting{get_settings().rpi_libcamera_awb_index,cb_awb}});
    auto cb_metring=[this](std::string,int value) {
      return set_rpi_libcamera_metering_index(value);
    };
    ret.push_back(openhd::Setting{"METERING_MODE_LC",openhd::IntSetting{get_settings().rpi_libcamera_metering_index,cb_metring}});
    auto cb_exposure=[this](std::string,int value) {
      return set_rpi_libcamera_exposure_index(value);
    };
    ret.push_back(openhd::Setting{"EXPOSURE_MODE_LC",openhd::IntSetting{get_settings().rpi_libcamera_exposure_index,cb_exposure}});
    auto cb_shutter=[this](std::string,int value) {
      return set_rpi_libcamera_shutter_microseconds(value);
    };
    ret.push_back(openhd::Setting{"SHUTTER_US_LC",openhd::IntSetting{get_settings().rpi_libcamera_shutter_microseconds,cb_shutter}});
  }
  {
    // Supported by all cameras, since it has actually nothing to do with the camera, only the link -
    // but for the user, it is more of a camera setting
    auto cb_encryption=[this](std::string,int value) {
      return set_encryption_enable(value);
    };
    ret.push_back(openhd::Setting{"HIGH_ENCRYPTION",openhd::IntSetting{get_settings().enable_ultra_secure_encryption,cb_encryption}});
  }
  return ret;
}

static nlohmann::json cameras_to_json(const std::vector<Camera> &cameras) {
  nlohmann::json j;
  for (const auto &camera : cameras) {
    nlohmann::json _camera = camera;
    j.push_back(_camera);
  }
  return j;
}

static constexpr auto CAMERA_MANIFEST_FILENAME = "/tmp/camera_manifest";

void write_camera_manifest(const std::vector<Camera> &cameras) {
    auto manifest = cameras_to_json(cameras);
    OHDFilesystemUtil::write_file(CAMERA_MANIFEST_FILENAME,manifest.dump(4));
}

void startup_fix_common_issues(std::vector<std::shared_ptr<CameraHolder>> &camera_holders) {
    if(camera_holders.empty()){
        openhd::log::get_default()->warn("at least 1 camera is a hard requirement");
        return;
    }
    // We always enable streaming for camera(s) on startup, to avoid the case where a user disables streaming for a camera,
    // and then forgets about it & reboots and the premise "always an image on startup with a working setup" is suddenly not true anymore.
    for(auto & camera_holder : camera_holders){
        camera_holder->unsafe_get_settings().enable_streaming= true;
        camera_holder->persist();
    }
    // And we disable recording on boot, to not accidentally fill up storage (relates to the new start stop recording widget)
    // June 20: Not needed anymore, since we stop recording when storage is running full and have start / stop recording when armed
    /*for(auto & camera_holder : camera_holders){
      camera_holder->unsafe_get_settings().air_recording= Recording::DISABLED;
      camera_holder->persist();
    }*/
    /*camera_holders.at(0)->unsafe_get_settings().enable_streaming= true;
    camera_holders.at(0)->persist();
    for(int i=1;i<camera_holders.size();i++){
      camera_holders.at(i)->unsafe_get_settings().enable_streaming = false;
      camera_holders.at(i)->persist();
    }*/
}
