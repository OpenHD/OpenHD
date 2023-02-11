//
// Created by consti10 on 18.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_

#include "camera.hpp"
#include "camera_settings.hpp"
#include "openhd_action_handler.hpp"
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
    public openhd::settings::PersistentSettings<CameraSettings>,
    // changes requested by the mavlink parameter protocol are propagated through lambda callbacks
    public openhd::ISettingsComponent{
 public:
  explicit CameraHolder(Camera camera,std::shared_ptr<openhd::ActionHandler> opt_action_handler= nullptr):
       m_camera(std::move(camera)),
       openhd::settings::PersistentSettings<CameraSettings>(openhd::get_video_settings_directory()){
    // read previous settings or create default ones
    init();
  }
  [[nodiscard]] const Camera& get_camera()const{
    return m_camera;
  }
  // Settings hacky begin
  std::vector<openhd::Setting> get_all_settings() override{
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
        openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{recording_to_int(get_settings().air_recording),c_recording}},
        // for debugging
        openhd::create_read_only_string("V_CAM_TYPE",camera_type_to_string(m_camera.type)),
        openhd::create_read_only_string("V_CAM_NAME",get_name_mavlink_safe()),
    };
    if(m_camera.type==CameraType::IP){
      auto cb_ip_cam_url=[this](std::string,std::string value){
        return set_ip_cam_url(value);
      };
      ret.push_back(openhd::Setting{"V_IP_CAM_URL",openhd::StringSetting {get_settings().ip_cam_url,cb_ip_cam_url}});
    }
    if(m_camera.type==CameraType::UVC){
      auto cb=[this](std::string,int value){
        if(!openhd::validate_yes_or_no(value))return false;
        unsafe_get_settings().usb_uvc_force_sw_encoding = value;
        persist();
        return true;
      };
      ret.push_back(openhd::Setting{"V_FORCE_SW_ENC",openhd::IntSetting {get_settings().usb_uvc_force_sw_encoding,cb}});
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
      ret.push_back(openhd::Setting{"V_MJPEG_QUALITY",openhd::IntSetting{get_settings().mjpeg_quality_percent,c_mjpeg_quality_percent}});
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
    if(m_camera.type==CameraType::RPI_CSI_LIBCAMERA){
      // r.n we only write the sensor name for cameras detected via libcamera
	  ret.push_back(openhd::create_read_only_string("V_CAM_SENSOR",m_camera.sensor_name));
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
    if(m_camera.type==CameraType::RPI_CSI_MMAL){
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
    return ret;
  }
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
    if(recording_enable==0 || recording_enable==1){
      const auto wanted_recording= recording_from_int(recording_enable);
      unsafe_get_settings().air_recording=wanted_recording;
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
  bool set_ip_cam_url(std::string value){
    if(value.size()>50)return false;
    unsafe_get_settings().ip_cam_url=value;
    persist();
    return true;
  }
  // 15 char mavlink string limit
  std::string get_name_mavlink_safe()const{
    auto tmp=m_camera.name;
    if(tmp.size()>15)tmp.resize(15);
    return tmp;
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
    // TODO: Hopefully "unique enough" now - but there might be issues if the name is
    // not properly parsed (especially for USB camera(s))
    return fmt::format("{}_{}_{}.json",m_camera.index, camera_type_to_string(m_camera.type),m_camera.name);
  }
  [[nodiscard]] CameraSettings create_default()const override{
    auto ret=CameraSettings{};
    if(m_camera.type==CameraType::RPI_CSI_MMAL || m_camera.type==CameraType::RPI_CSI_LIBCAMERA){
      ret.streamed_video_format.width=1280;
      ret.streamed_video_format.height=720;
      ret.streamed_video_format.framerate=30;
    }
    if(m_camera.type==CameraType::RPI_VEYE_CSI_V4l2){
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

static void startup_fix_common_issues(std::vector<std::shared_ptr<CameraHolder>>& camera_holders){
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
  for(auto & camera_holder : camera_holders){
    camera_holder->unsafe_get_settings().air_recording= Recording::DISABLED;
    camera_holder->persist();
  }
  /*camera_holders.at(0)->unsafe_get_settings().enable_streaming= true;
  camera_holders.at(0)->persist();
  for(int i=1;i<camera_holders.size();i++){
    camera_holders.at(i)->unsafe_get_settings().enable_streaming = false;
    camera_holders.at(i)->persist();
  }*/
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
