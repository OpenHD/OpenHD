//
// Created by consti10 on 18.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_

#include "camera.hpp"
#include "camera2.h"
#include "camera_settings.hpp"
#include "openhd_action_handler.h"
#include "openhd_bitrate_conversions.hpp"
#include "openhd_settings_directories.hpp"
#include "openhd_settings_imp.hpp"
#include "openhd_settings_persistent.h"

// Holds the immutable (camera) and mutable (camera_settings) information about a camera
// Camera Holder is used to differentiate between
// 1) immutable information (camera) and
// 2) mutable camera settings.
// Changes in the camera settings are propagated through this class.
class CameraHolder:
    // persistence via JSON
    public openhd::PersistentSettings<CameraSettings>,
    // changes requested by the mavlink parameter protocol are propagated through lambda callbacks
    public openhd::ISettingsComponent{
 public:
  explicit CameraHolder(Camera camera):
       m_camera(std::move(camera)),
       openhd::PersistentSettings<CameraSettings>(openhd::get_video_settings_directory()){
    // read previous settings or create default ones
    init();
  }
  [[nodiscard]] const Camera& get_camera()const{
    return m_camera;
  }
  // Settings hacky begin
  std::vector<openhd::Setting> get_all_settings() override;
  bool set_enable_streaming(int enable){
    if(!openhd::validate_yes_or_no(enable))return false;
    unsafe_get_settings().enable_streaming=static_cast<bool>(enable);
    persist();
    return true;
  }
  // it is only possible to validate setting the video width,height and fps
  // if we do them together
  bool set_video_width_height_framerate(int width,int height,int framerate){
    if(!openhd::validate_video_width_height_fps(width,height,framerate)){
      return false;
    }
    unsafe_get_settings().streamed_video_format.width=width;
    unsafe_get_settings().streamed_video_format.height=height;
    unsafe_get_settings().streamed_video_format.framerate=framerate;
    persist();
    return true;
  }
  bool set_video_codec(int codec){
    if(!openhd::validate_video_codec(codec)){
      return false;
    }
    unsafe_get_settings().streamed_video_format.videoCodec=video_codec_from_int(codec);
    persist();
    return true;
  }
  bool set_video_bitrate(int bitrate_mbits){
    if(!openhd::validate_bitrate_mbits(bitrate_mbits)){
      return false;
    }
    unsafe_get_settings().h26x_bitrate_kbits = mbits_to_kbits_per_second(bitrate_mbits);
    persist();
    return true;
  }
  bool set_air_recording(int recording_enable){
    if(OHDFilesystemUtil::get_remaining_space_in_mb()<MINIMUM_AMOUNT_FREE_SPACE_FOR_AIR_RECORDING_MB){
      openhd::log::get_default()->warn("Not enough free space available");
      return false;
    }
    if(get_settings().air_recording==AIR_RECORDING_AUTO_ARM_DISARM && (recording_enable==AIR_RECORDING_ON || recording_enable==AIR_RECORDING_OFF)){
      openhd::log::get_default()->warn("Auto record on arm disabled");
    }
    if(recording_enable==AIR_RECORDING_OFF || recording_enable==AIR_RECORDING_ON || recording_enable==AIR_RECORDING_AUTO_ARM_DISARM){
      unsafe_get_settings().air_recording=recording_enable;
      persist();
      return true;
    }
    return false;
  }
  bool set_camera_rotation(int value){
    if(!m_camera.supports_rotation())return false;
    if(!openhd::validate_camera_rotation(value)){
      return false;
    }
    unsafe_get_settings().camera_rotation_degree=value;
    persist();
    return true;
  }
  bool set_keyframe_interval(int value){
    if(!openhd::validate_rpi_keyframe_interval(value))return false;
    unsafe_get_settings().h26x_keyframe_interval =value;
    persist();
    return true;
  }
  bool set_intra_refresh_type(int value){
    if(!openhd::validate_rpi_intra_refresh_type(value))return false;
    unsafe_get_settings().h26x_intra_refresh_type =value;
    persist();
    return true;
  }
  bool set_camera_awb(int value){
    if(!m_camera.supports_awb())return false;
    if(!openhd::validate_rpi_awb_mode(value)){
      return false;
    }
    unsafe_get_settings().awb_mode=value;
    persist();
    return true;
  }
  bool set_camera_exposure(int value){
    if(!m_camera.supports_exp())return false;
    if(!openhd::validate_rpi_exp_mode(value)){
      return false;
    }
    unsafe_get_settings().exposure_mode=value;
    persist();
    return true;
  }
  bool set_mjpeg_quality_percent(int value){
    if(!openhd::validate_mjpeg_quality_percent(value)){
      return false;
    }
    unsafe_get_settings().mjpeg_quality_percent=value;
    persist();
    return true;
  }
  bool set_vertical_flip(int value){
    if(!openhd::validate_yes_or_no(value))return false;
    unsafe_get_settings().vertical_flip=static_cast<bool>(value);
    persist();
    return true;
  }
  bool set_horizontal_flip(int value){
    if(!openhd::validate_yes_or_no(value))return false;
    unsafe_get_settings().horizontal_flip=static_cast<bool>(value);
    persist();
    return true;
  }
  bool set_brightness(int value){
    if(!openhd::validate_rpi_brightness(value))return false;
    unsafe_get_settings().brightness_percentage=value;
    persist();
    return true;
  }
  bool set_rpi_rpicamsrc_iso(int value){
    if(!openhd::validate_rpi_rpicamsrc_iso(value))return false;
    unsafe_get_settings().rpi_rpicamsrc_iso=value;
    persist();
    return true;
  }
  bool set_rpi_rpicamsrc_metering_mode(int value){
    if(!openhd::validate_rpi_rpicamsrc_metering_mode(value)) return false;
    unsafe_get_settings().rpi_rpicamsrc_metering_mode=value;
    persist();
    return true;
  }
  bool set_rpi_libcamera_sharpness_as_int(int value){
      if(!openhd::validate_rpi_libcamera_sharpness_as_int(value))return false;
      unsafe_get_settings().rpi_libcamera_sharpness_as_int=value;
      persist(true);
      return true;
  }
  bool set_rpi_libcamera_contrast_as_int(int value){
      if(!openhd::validate_rpi_libcamera_contrast_as_int(value))return false;
      unsafe_get_settings().rpi_libcamera_contrast_as_int=value;
      persist(true);
      return true;
  }
  bool set_rpi_libcamera_saturation_as_int(int value){
      if(!openhd::validate_rpi_libcamera_saturation_as_int(value))return false;
      unsafe_get_settings().rpi_libcamera_saturation_as_int=value;
      persist(true);
      return true;
  }
  bool set_rpi_libcamera_ev_value(int value){
      if(!openhd::validate_rpi_libcamera_ev_value(value))return false;
      unsafe_get_settings().rpi_libcamera_ev_value=value;
      persist(true);
      return true;
  }
  bool set_rpi_libcamera_denoise_index(int value){
    if(!openhd::validate_rpi_libcamera_doenise_index(value))return false;
    unsafe_get_settings().rpi_libcamera_denoise_index=value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_awb_index(int value){
    if(!openhd::validate_rpi_libcamera_awb_index(value))return false;
    unsafe_get_settings().rpi_libcamera_awb_index=value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_metering_index(int value){
    if(!openhd::validate_rpi_libcamera_metering_index(value))return false;
    unsafe_get_settings().rpi_libcamera_metering_index=value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_exposure_index(int value){
    if(!openhd::validate_rpi_libcamera_exposure_index(value))return false;
    unsafe_get_settings().rpi_libcamera_exposure_index=value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_shutter_microseconds(int value){
    if(!openhd::validate_rpi_libcamera_shutter_microseconds(value))return false;
    unsafe_get_settings().rpi_libcamera_shutter_microseconds=value;
    persist(true);
    return true;
  }
  bool set_ip_cam_url(std::string value){
    if(value.size()>50)return false;
    unsafe_get_settings().ip_cam_url=value;
    persist();
    return true;
  }
  bool set_encryption_enable(int enable){
      if(!openhd::validate_yes_or_no(enable))return false;
      unsafe_get_settings().enable_ultra_secure_encryption=enable;
      // Doesn't need restart of the camera pipeline, weather to encrypt or not is passed per frame to wb
      persist(false);
      return true;
  }
  bool set_custom_script_param0(int value){
      unsafe_get_settings().custom_script_value0=value;
      persist(false);
      return true;
  }
  bool set_custom_script_param1(int value){
      unsafe_get_settings().custom_script_value1=value;
      persist(false);
      return true;
  }
  // The CSI to HDMI adapter has an annoying bug where it actually doesn't allow changing the framerate but takes whatever the host provides
  // (e.g. the hdmi card). Util to check if we need to apply the "reduce bitrate by half"
  // NOTE: This is not completely correct - it assumes the provider (e.g. gopro) always gives 60fps
  // and in case the user selects 720p@49fps for example, the bitrate is too low.
  // However, rather be too low than too high - the user can always go higher if he needs to.
  bool requires_half_bitrate_workaround()const{
    return m_camera.type==CameraType::RPI_CSI_MMAL && m_camera.rpi_csi_mmal_is_csi_to_hdmi && get_settings().streamed_video_format.framerate!=60;
  }
  // Settings hacky end
 private:
  // Camera info is immutable
  const Camera m_camera;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    return m_camera.get_unique_settings_filename();
  }
  std::optional<CameraSettings> impl_deserialize(const std::string& file_as_string)const override;
  std::string imp_serialize(const CameraSettings& data)const override;
  [[nodiscard]] CameraSettings create_default()const override{
    auto ret=CameraSettings{};
    if(m_camera.type==CameraType::RPI_CSI_MMAL || m_camera.type==CameraType::RPI_CSI_LIBCAMERA){
      ret.streamed_video_format.width=1280;
      ret.streamed_video_format.height=720;
      ret.streamed_video_format.framerate=30;
    }
    if(m_camera.type==CameraType::RPI_CSI_VEYE_V4l2){
      // most veye cameras can only do 1080p30, nothing else
      ret.streamed_video_format.width=1920;
      ret.streamed_video_format.height=1080;
      ret.streamed_video_format.framerate=30;
    }
    if(m_camera.type==CameraType::UVC){
      // We need to find a resolution / framerate format that is supported by the camera, note that OpenHD always defaults to h264
      const auto opt_h264_endpoint= get_endpoint_supporting_codec(m_camera.v4l2_endpoints,VideoCodec::H264);
      if(opt_h264_endpoint.has_value()){
        // Just pick the first one from the array
        assert(!opt_h264_endpoint.value().formats_h264.empty());
        const auto format=opt_h264_endpoint.value().formats_h264.at(0);
        openhd::log::get_default()->debug("Selecting {} as default",format.debug());
        ret.streamed_video_format.width=format.width;
        ret.streamed_video_format.height=format.height;
        ret.streamed_video_format.framerate=format.fps;
        return ret;
      }
      const auto opt_raw_endpoint= get_endpoint_supporting_raw(m_camera.v4l2_endpoints);
      if(opt_raw_endpoint.has_value()){
        // Just pick the first one from the array
        assert(!opt_raw_endpoint.value().formats_raw.empty());
        const auto format=opt_raw_endpoint.value().formats_raw.at(0);
        openhd::log::get_default()->debug("Selecting {} as default",format.debug());
        ret.streamed_video_format.width=format.width;
        ret.streamed_video_format.height=format.height;
        ret.streamed_video_format.framerate=format.fps;
        return ret;
      }
      openhd::log::get_default()->warn("Cannot find valid default resolution for USB camera");
    }
    return ret;
  }
};

static std::shared_ptr<CameraHolder> createDummyCamera2(){
  return std::make_shared<CameraHolder>(createDummyCamera());
}

void startup_fix_common_issues(std::vector<std::shared_ptr<CameraHolder>>& camera_holders);

void write_camera_manifest(const std::vector<Camera> &cameras);

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
