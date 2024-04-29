//
// Created by consti10 on 19.09.23.
//
#include "camera_holder.h"

#include "include_json.hpp"
#include "openhd_spdlog_include.h"

NLOHMANN_JSON_SERIALIZE_ENUM(VideoCodec, {
                                             {VideoCodec::H264, "h264"},
                                             {VideoCodec::H265, "h265"},
                                         });

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoFormat, videoCodec, width, height,
                                   framerate)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    CameraSettings, enable_streaming, streamed_video_format, h26x_bitrate_kbits,
    h26x_keyframe_interval, h26x_intra_refresh_type, h26x_num_slices,
    air_recording, camera_rotation_degree, openhd_flip, openhd_brightness,
    openhd_sharpness, openhd_saturation, openhd_contrast,
    // rpi libcamera specific IQ params begin
    rpi_libcamera_ev_value, rpi_libcamera_denoise_index,
    rpi_libcamera_awb_index, rpi_libcamera_metering_index,
    rpi_libcamera_exposure_index, rpi_libcamera_shutter_microseconds,
    // rpi libcamera specific IQ params end
    force_sw_encode, enable_ultra_secure_encryption,
    infiray_custom_control_zoom_absolute_colorpalete)

std::optional<CameraSettings> CameraHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<CameraSettings>(file_as_string);
}

std::string CameraHolder::imp_serialize(const CameraSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

std::vector<openhd::Setting> CameraHolder::get_all_settings() {
  std::vector<openhd::Setting> ret;
  if (!OHDPlatform::instance().is_x20()) {
    auto c_width_height_framerate = [this](std::string, std::string value) {
      auto tmp_opt = parse_video_format(value);
      if (tmp_opt.has_value()) {
        const auto &tmp = tmp_opt.value();
        return set_video_width_height_framerate(tmp.width_px, tmp.height_px,
                                                tmp.fps);
      }
      return false;
    };
    // Width, height and FPS are done together
    const auto format_string = openhd::video_format_from_int_values(
        get_settings().streamed_video_format.width,
        get_settings().streamed_video_format.height,
        get_settings().streamed_video_format.framerate);
    ret.push_back(openhd::Setting{
        "RESOLUTION_FPS",
        openhd::StringSetting{format_string, c_width_height_framerate}});
  }
  if (!OHDPlatform::instance().is_x20()) {
    auto c_codec = [this](std::string, int value) {
      return set_video_codec(value);
    };
    ret.push_back(openhd::Setting{
        "VIDEO_CODEC",
        openhd::IntSetting{
            video_codec_to_int(get_settings().streamed_video_format.videoCodec),
            c_codec}});
  }
  {
    // Supported by all cameras, since it has actually nothing to do with the
    // camera, only the link - but for the user, it is more of a camera setting
    auto cb_encryption = [this](std::string, int value) {
      return set_encryption_enable(value);
    };
    ret.push_back(openhd::Setting{
        "HIGH_ENCRYPTION",
        openhd::IntSetting{get_settings().enable_ultra_secure_encryption,
                           cb_encryption}});
  }
  const bool supports_rotation_vflip_hflip =
      m_camera.requires_rpi_libcamera_pipeline() ||
      m_camera.requires_rpi_mmal_pipeline() ||
      m_camera.x20_supports_basic_iq_params() ||
      m_camera.requires_rockchip_mpp_pipeline() ||
      m_camera.camera_type == X_CAM_TYPE_DEVELOPMENT_FILESRC;
  if (supports_rotation_vflip_hflip) {
    auto c_rotation = [this](std::string, int value) {
      return set_camera_rotation(value);
    };
    ret.push_back(openhd::Setting{
        "ROTATION_DEG",
        openhd::IntSetting{get_settings().camera_rotation_degree, c_rotation}});
  }
  if (supports_rotation_vflip_hflip) {
    auto c_openhd_flip = [this](std::string, int value) {
      return set_openhd_flip(value);
    };
    ret.push_back(openhd::Setting{
        "ROTATION_FLIP",
        openhd::IntSetting{get_settings().openhd_flip, c_openhd_flip}});
  }
  if (true) {
    auto c_enable_streaming = [this](std::string, int value) {
      return set_enable_streaming(value);
    };
    auto c_recording = [this](std::string, int value) {
      return set_air_recording(value);
    };
    ret.push_back(openhd::Setting{
        "STREAMING_E", openhd::IntSetting{get_settings().enable_streaming,
                                          c_enable_streaming}});
    ret.push_back(openhd::Setting{
        "AIR_RECORDING_E",
        openhd::IntSetting{get_settings().air_recording, c_recording}});
  }
  // if(m_camera.sensor_name!="unknown"){
  //   ret.emplace_back(openhd::create_read_only_string("V_CAM_SENSOR",m_camera.sensor_name));
  // }
  if (!OHDPlatform::instance().is_x20()) {
    auto cb = [this](std::string, int value) {
      if (!openhd::validate_yes_or_no(value)) return false;
      unsafe_get_settings().force_sw_encode = value;
      persist();
      return true;
    };
    ret.push_back(openhd::Setting{
        "FORCE_SW_ENC",
        openhd::IntSetting{get_settings().force_sw_encode, cb}});
  }
  if (true) {
    // NOTE: OpenHD stores the bitrate in kbit/s, but for now we use MBit/s for
    // the setting (Since that is something a normal user can make more sense
    // of) and just multiply the value in the callback
    auto c_bitrate = [this](std::string, int value) {
      return set_video_bitrate(value);
    };
    ret.push_back(openhd::Setting{
        "BITRATE_MBITS",
        openhd::IntSetting{
            static_cast<int>(get_settings().h26x_bitrate_kbits / 1000),
            c_bitrate}});
  }
  if (true) {
    auto c_keyframe_interval = [this](std::string, int value) {
      return set_keyframe_interval(value);
    };
    ret.push_back(openhd::Setting{
        "KEYFRAME_I", openhd::IntSetting{get_settings().h26x_keyframe_interval,
                                         c_keyframe_interval}});
  }
  if (true) {  // Always show intra, on libcamera without sw encode it
               // unfortunately is 'just not mapped' and ignored.
    auto c_intra_refresh_type = [this](std::string, int value) {
      return set_intra_refresh_type(value);
    };
    ret.push_back(openhd::Setting{
        "INTRA_REFRESH",
        openhd::IntSetting{get_settings().h26x_intra_refresh_type,
                           c_intra_refresh_type}});
    auto c_h26x_num_slices = [this](std::string, int value) {
      return set_h26x_num_slices(value);
    };
    ret.push_back(openhd::Setting{
        "N_SLICES",
        openhd::IntSetting{get_settings().h26x_num_slices, c_h26x_num_slices}});
  }
  // right now only supported by libcamera and (partially) x20
  const bool SUPPORTS_OPENHD_IQ = m_camera.requires_rpi_libcamera_pipeline() ||
                                  m_camera.x20_supports_basic_iq_params();
  if (SUPPORTS_OPENHD_IQ) {
    auto cb_sharpness = [this](std::string, int value) {
      return set_openhd_sharpness(value);
    };
    ret.push_back(openhd::Setting{
        "SHARPNESS",
        openhd::IntSetting{get_settings().openhd_sharpness, cb_sharpness}});
    auto cb_contrast = [this](std::string, int value) {
      return set_openhd_contrast(value);
    };
    ret.push_back(openhd::Setting{
        "CONTRAST",
        openhd::IntSetting{get_settings().openhd_contrast, cb_contrast}});
    auto cb_saturation = [this](std::string, int value) {
      return set_openhd_saturation(value);
    };
    ret.push_back(openhd::Setting{
        "SATURATION",
        openhd::IntSetting{get_settings().openhd_saturation, cb_saturation}});
    auto c_brightness = [this](std::string, int value) {
      return set_openhd_brightness(value);
    };
    ret.push_back(openhd::Setting{
        "BRIGHTNESS",
        openhd::IntSetting{get_settings().openhd_brightness, c_brightness}});
  }
  const bool SUPPORTS_IQ = m_camera.requires_rpi_libcamera_pipeline();
  // These are rpi libcamera specific image quality settings
  if (SUPPORTS_IQ) {
    auto cb_ev = [this](std::string, int value) {
      return set_rpi_libcamera_ev_value(value);
    };
    ret.push_back(openhd::Setting{
        "EXPOSURE_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_ev_value, cb_ev}});

    auto cb_denoise = [this](std::string, int value) {
      return set_rpi_libcamera_denoise_index(value);
    };
    ret.push_back(openhd::Setting{
        "DENOISE_INDEX_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_denoise_index,
                           cb_denoise}});
    auto cb_awb = [this](std::string, int value) {
      return set_rpi_libcamera_awb_index(value);
    };
    ret.push_back(openhd::Setting{
        "AWB_MODE_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_awb_index, cb_awb}});
    auto cb_metring = [this](std::string, int value) {
      return set_rpi_libcamera_metering_index(value);
    };
    ret.push_back(openhd::Setting{
        "METERING_MODE_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_metering_index,
                           cb_metring}});
    auto cb_exposure = [this](std::string, int value) {
      return set_rpi_libcamera_exposure_index(value);
    };
    ret.push_back(openhd::Setting{
        "EXPOSURE_MODE_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_exposure_index,
                           cb_exposure}});
    auto cb_shutter = [this](std::string, int value) {
      return set_rpi_libcamera_shutter_microseconds(value);
    };
    ret.push_back(openhd::Setting{
        "SHUTTER_US_LC",
        openhd::IntSetting{get_settings().rpi_libcamera_shutter_microseconds,
                           cb_shutter}});
  }
  if (m_camera.is_camera_type_usb_infiray()) {
    auto cb_infiray = [this](std::string, int value) {
      return set_infiray_custom_control_zoom_absolute_colorpalete(value);
    };
    ret.push_back(openhd::Setting{
        "COLOR_PALETE",
        openhd::IntSetting{
            get_settings().infiray_custom_control_zoom_absolute_colorpalete,
            cb_infiray}});
  }
  return ret;
}

bool CameraHolder::set_air_recording(int recording_enable) {
  if (recording_enable != AIR_RECORDING_OFF &&
      OHDFilesystemUtil::get_remaining_space_in_mb() <
          MINIMUM_AMOUNT_FREE_SPACE_FOR_AIR_RECORDING_MB) {
    openhd::log::get_default()->warn("Not enough free space available");
    return false;
  }
  if (get_settings().air_recording == AIR_RECORDING_AUTO_ARM_DISARM &&
      (recording_enable == AIR_RECORDING_ON ||
       recording_enable == AIR_RECORDING_OFF)) {
    openhd::log::get_default()->warn("Auto record on arm disabled");
  }
  if (recording_enable == AIR_RECORDING_OFF ||
      recording_enable == AIR_RECORDING_ON ||
      recording_enable == AIR_RECORDING_AUTO_ARM_DISARM) {
    unsafe_get_settings().air_recording = recording_enable;
    persist();
    return true;
  }
  return false;
}

void CameraHolder::check_remaining_space_air_recording(bool call_callback) {
  if (OHDFilesystemUtil::get_remaining_space_in_mb() >
      MINIMUM_AMOUNT_FREE_SPACE_FOR_AIR_RECORDING_MB) {
    return;
  }
  openhd::log::get_default()->warn("Not enough free space available");
  if (get_settings().air_recording != AIR_RECORDING_OFF) {
    openhd::log::get_default()->debug("Disabling air recording");
    unsafe_get_settings().air_recording = AIR_RECORDING_OFF;
    persist(call_callback);
  }
}
