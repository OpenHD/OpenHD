//
// Created by consti10 on 18.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_

#include "openhd-settings2.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-settings.hpp"
#include "openhd_bitrate_conversions.hpp"
#include "mavlink_settings/ISettingsComponent.hpp"

#include "camera.hpp"
#include "camera_settings.hpp"

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
       m_camera(std::move(camera)),m_opt_action_handler(std::move(opt_action_handler)),
       openhd::settings::PersistentSettings<CameraSettings>(get_video_settings_directory()){
    // read previous settings or create default ones
    init();
    // dirty, propagate changes to the video codec back to the link (ohd_interface)
    if(m_opt_action_handler){
      m_opt_action_handler->action_set_video_codec_handle(video_codec_to_int(get_settings().streamed_video_format.videoCodec));
    }
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
    // This is not a setting (cannot be changed) but rather a read-only param, but repurposing the settings here was the easiest
    auto c_read_only_param=[this](std::string,std::string value){
      return false;
    };
    std::vector<openhd::Setting> ret={
        openhd::Setting{"V_E_STREAMING",openhd::IntSetting{get_settings().enable_streaming,c_enable_streaming}},
        openhd::Setting{"VIDEO_CODEC",openhd::IntSetting{video_codec_to_int(get_settings().streamed_video_format.videoCodec), c_codec}},
        openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{recording_to_int(get_settings().air_recording),c_recording}},
        // for debugging
        openhd::Setting{"V_CAM_TYPE",openhd::StringSetting { get_short_name(),c_read_only_param}},
    };
    if(m_camera.type==CameraType::IP){
      auto cb_ip_cam_url=[this](std::string,std::string value){
        return set_ip_cam_url(value);
      };
      ret.push_back(openhd::Setting{"V_IP_CAM_URL",openhd::StringSetting {get_settings().ip_cam_url,cb_ip_cam_url}});
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
      ret.push_back(openhd::Setting{"V_CAM_SENSOR",openhd::StringSetting{m_camera.sensor_name,c_read_only_param}});
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
    if(m_opt_action_handler){
      m_opt_action_handler->action_set_video_codec_handle(codec);
    }
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
  bool set_ip_cam_url(std::string value){
    if(value.size()>50)return false;
    unsafe_get_settings().ip_cam_url=value;
    persist();
    return true;
  }
  // Settings hacky end
 private:
  // Camera info is immutable
  const Camera m_camera;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    std::stringstream ss;
    // TODO: r.n not unique enough, we need to be unique per model,too - e.g. a user
    // might connect a different USB camera, where we'd need a different unique ID for
    ss<<(static_cast<int>(m_camera.index))<<"_"<<camera_type_to_string(m_camera.type);
    return ss.str();
  }
  [[nodiscard]] CameraSettings create_default()const override{
    auto ret=CameraSettings{};
    if(m_camera.type==CameraType::RPI_VEYE_CSI_MMAL){
      // Veye cannot do 640x480@30 by default, this is the next lower possible
      // (TODO it should do 720p but for some reason doesn't)
      ret.streamed_video_format.width=1920;
      ret.streamed_video_format.height=1080;
      ret.streamed_video_format.framerate=30;
    }
    return ret;
  }
  [[nodiscard]] std::string get_short_name()const{
    return camera_type_to_string(m_camera.type);
  }
  // Where the settings (json) for each discovered camera are stored
  static std::string get_video_settings_directory(){
    return std::string(BASE_PATH)+std::string("video/");
  }
};

static std::shared_ptr<CameraHolder> createDummyCamera2(){
  return std::make_shared<CameraHolder>(createDummyCamera());
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
