#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "camera_enums.hpp"
#include "discovered_camera.hpp"
#include "mavlink_settings/ISettingsComponent.hpp"
#include "openhd-action-handler.hpp"
#include "openhd-settings.hpp"
#include "openhd-settings2.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"
#include "v_validate_settings.h"

static constexpr int DEFAULT_BITRATE_KBITS = 5000;
// Quite low, at 30fps a keyframe interval of 15 means a single lost keyframe results in max. 500ms
// of interruption (assuming the next keyframe is properly received).
// Noe that if you double the FPS, you can also double the keyframe interval if you want to keep those interruptions
// at roughly 500ms
static constexpr int DEFAULT_KEYFRAME_INTERVAL = 15;
static constexpr int DEFAULT_MJPEG_QUALITY_PERCENT = 50;

static constexpr int DEFAULT_RECORDING_KBITS = 10000;
static constexpr int DEFAULT_RECORDING_QP = 26;
static constexpr RateControlMode DEFAULT_RC_MODE = RateControlMode::RC_CBR;

// NOTE: I am not completely sure, but the more common approach seems to multiply / divide by 1000
// When converting mBit/s to kBit/s or the other way arund
// some encoders take bits per second instead of kbits per second
static int kbits_to_bits_per_second(int kbit_per_second){
  return kbit_per_second*1000;
}
static int kbits_to_mbits_per_second(int kbits_per_second){
  return kbits_per_second/1000;
}
static int mbits_to_kbits_per_second(int mbits_per_second){
  return mbits_per_second*1000;
}
// Return true if the bitrate is considered sane, false otherwise
static bool check_bitrate_sane(const int bitrateKBits) {
  if (bitrateKBits <= 100 || bitrateKBits > (1000 * 1000 * 50)) {
    return false;
  }
  return true;
}

// User-selectable camera options
// These values are settings that can change dynamically at run time
// (non-deterministic)
struct CameraSettings {
  // Enable / Disable streaming for this camera
  // This can be usefully for debugging, but also when the there is suddenly a really high interference,
  // and the user wants to fly home without video, using only telemetry / HUD.
  // Default to true, otherwise we'd have conflicts with the "always a picture without changing any settings" paradigm.
  bool enable_streaming=true;
  // The video format selected by the user. If the user sets a video format that
  // isn't supported (for example, he might select h264|1920x1080@120 but the
  // camera can only do 60fps) The stream might default to the first available
  // video format. We generally default to h264|640x480@30, except for the rare case when
  // a camera cannot do this exact resolution (e.g. veye). In this case, these default params are overridden
  // the first time settings are created for a specific discovered camera (see create_default() below)
  VideoFormat streamed_video_format{VideoCodec::H264, 640, 480, 30};
  // The settings below can only be implemented on a "best effort" manner -
  // changing them does not necessarily mean the camera supports changing them. Unsupported settings have to
  // be ignored during pipeline construction
  // In general, we only try to expose these values as mavlink parameters if the camera supports them, to not
  // confuse the user.
  //
  // ----------------------------------------------------------------------------------------------------------
  //
  // The bitrate the generated stream should have. Note that not all cameras /
  // encoders support a constant bitrate, and not all encoders support all
  // bitrates, especially really low ones. How an encoder handles a specific constant bitrate is vendor specific.
  // Note that we always use a constant bitrate in OpenHD, since it is the only way to properly adjust the bitrate
  // depending on the link quality (once we have that wired up).
  // Also note that his param is for h264 and h265 - mjpeg normally does not have a bitrate param, only a quality param.
  int h26x_bitrate_kbits = DEFAULT_BITRATE_KBITS;
  // Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe , else positive values up to 2147483647
  // note that with 0 and/or sometimes -1, you can create issues like no decoded image at all,
  // since wifibroadcast relies on keyframes in regular intervals.
  // Also, some camera(s) might use a different mapping in regard to the keyframe interval than what's defined here,
  // supporting them needs different setting validation methods.
  // only valid for h264 / h265, mjpeg has no keyframe interval
  int h26x_keyframe_interval =DEFAULT_KEYFRAME_INTERVAL;
  // Type of Intra Refresh to use, -1 to disable intra refresh. R.n only supported on gst-rpicamsrc.
  // see gst-rpicamsrc for more info.
  int h26x_intra_refresh_type =-1;
  // MJPEG has no bitrate parameter, only a "quality" param. This value is only used if the
  // user selected MJPEG as its video codec
  int mjpeg_quality_percent=DEFAULT_MJPEG_QUALITY_PERCENT;
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar
  std::string ip_cam_url;
  // enable/disable recording to file
  Recording air_recording=Recording::DISABLED;
  //
  // Below are params that most often only affect the ISP, not the encoder
  //
  // camera rotation, only supported on rpicamsrc at the moment
  // 0 nothing, 90° to the right, 180° to the right, 270° to the right
  int camera_rotation_degree=0;
  // horizontal / vertical flip, r.n only supported on rpicamsrc
  bool horizontal_flip= false;
  bool vertical_flip=false;
  // R.n only for rpi camera, see https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c
  int awb_mode=1; //default 1 (auto)
  int exposure_mode=1; //default 1 (auto)

  // only used on RK3588, dirty, r.n not persistent
  VideoFormat recordingFormat{VideoCodec::H264, 0, 0, 0}; // 0 means copy
  int recordingKBits = DEFAULT_RECORDING_KBITS;
  int recordingQP = DEFAULT_RECORDING_QP;
  RateControlMode recordingRCMode = DEFAULT_RC_MODE;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings,enable_streaming,
                                   streamed_video_format, h26x_bitrate_kbits,
                                   h26x_keyframe_interval, h26x_intra_refresh_type,mjpeg_quality_percent, ip_cam_url,air_recording,
                                   camera_rotation_degree,horizontal_flip,vertical_flip,
                                   awb_mode,exposure_mode)


static const std::string VIDEO_SETTINGS_DIRECTORY=std::string(BASE_PATH)+std::string("video/");

// Camera Holder is used to
// 1) Differentiate between immutable information (camera) and
// 2) mutable camera settings.
// Changes in the camera settings are propagated through this class.
class CameraHolder:public openhd::settings::PersistentSettings<CameraSettings>,
                     public openhd::ISettingsComponent{
 public:
  explicit CameraHolder(Camera camera,std::shared_ptr<openhd::ActionHandler> opt_action_handler= nullptr): m_camera(std::move(camera)),m_opt_action_handler(std::move(opt_action_handler)),
         openhd::settings::PersistentSettings<CameraSettings>(VIDEO_SETTINGS_DIRECTORY){
    init();
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
    // NOTE: OpenHD stores the bitrate in kbit/s, but for now we use MBit/s for the setting
    // (Since that is something a normal user can make more sense of)
    // and just multiply the value in the callback
    auto c_bitrate=[this](std::string,int value) {
      return set_video_bitrate(value);
    };
    auto c_keyframe_interval=[this](std::string,int value) {
      return set_keyframe_interval(value);
    };
    auto c_recording=[this](std::string,int value) {
      return set_air_recording(value);
    };
    auto c_mjpeg_quality_percent=[this](std::string,int value) {
      return set_mjpeg_quality_percent(value);
    };
    auto c_width_height_framerate=[this](std::string,std::string value){
      auto tmp_opt=openhd::parse_video_format(value);
      if(tmp_opt.has_value()){
        const auto& tmp=tmp_opt.value();
        return set_video_width_height_framerate(tmp.width_px,tmp.height_px,tmp.framerate);
      }
      return false;
    };
    // This is not a setting (cannot be changed) but rather a read-only param, but repurposing the settings here was the easiest
    auto c_read_only_param=[this](std::string,std::string value){
      return false;
    };
    std::vector<openhd::Setting> ret={
        openhd::Setting{"V_E_STREAMING",openhd::IntSetting{get_settings().enable_streaming,c_enable_streaming}},
        // Width, height and FPS are done together now (V_FORMAT)
        openhd::Setting{"V_FORMAT",openhd::StringSetting{
                                        openhd::video_format_from_int_values(get_settings().streamed_video_format.width,
                                                                             get_settings().streamed_video_format.height,
                                                                             get_settings().streamed_video_format.framerate),
                                        c_width_height_framerate
                                    }},
        openhd::Setting{"VIDEO_CODEC",openhd::IntSetting{video_codec_to_int(get_settings().streamed_video_format.videoCodec), c_codec}},
        openhd::Setting{"V_BITRATE_MBITS",openhd::IntSetting{static_cast<int>(get_settings().h26x_bitrate_kbits / 1000),c_bitrate}},
        openhd::Setting{"V_KEYFRAME_I",openhd::IntSetting{get_settings().h26x_keyframe_interval,c_keyframe_interval}},
        openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{recording_to_int(get_settings().air_recording),c_recording}},
        openhd::Setting{"V_MJPEG_QUALITY",openhd::IntSetting{get_settings().mjpeg_quality_percent,c_mjpeg_quality_percent}},
        // for debugging
        openhd::Setting{"V_CAM_TYPE",openhd::StringSetting { get_short_name(),c_read_only_param}},
    };
    if(m_camera.type==CameraType::Libcamera){
      // r.n we only write the sensor name for cameras detected via libcamera
      ret.push_back(openhd::Setting{"V_CAM_SENSOR",openhd::StringSetting{m_camera.sensor_name,c_read_only_param}});
    }
    if(m_camera.supports_rotation()){
      auto c_rotation=[this](std::string,int value) {
        return set_camera_rotation(value);
      };
      ret.push_back(openhd::Setting{"V_CAM_ROT_DEG",openhd::IntSetting{get_settings().camera_rotation_degree,c_rotation}});
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
    if(m_camera.type==CameraType::RaspberryPiCSI){
      auto c_vertical_flip=[this](std::string,int value) {
        return set_vertical_flip(value);
      };
      auto c_horizontal_flip=[this](std::string,int value) {
        return set_horizontal_flip(value);
      };
      auto c_intra_refresh_type=[this](std::string,int value) {
        return set_intra_refresh_type(value);
      };
      ret.push_back(openhd::Setting{"V_VERT_FLIP",openhd::IntSetting{get_settings().vertical_flip,c_vertical_flip}});
      ret.push_back(openhd::Setting{"V_HORIZ_FLIP",openhd::IntSetting{get_settings().horizontal_flip,c_horizontal_flip}});
      ret.push_back(openhd::Setting{"V_INTRA_REFRESH",openhd::IntSetting{get_settings().h26x_intra_refresh_type,c_intra_refresh_type}});
    }
    return ret;
  }
  bool set_enable_streaming(int enable){
    if(!openhd::validate_yes_or_no(enable))return false;
    unsafe_get_settings().enable_streaming=static_cast<bool>(enable);
    persist();
    return true;
  }
  bool set_video_width(int video_width){
    if(!openhd::validate_video_with(video_width)){
      return false;
    }
    unsafe_get_settings().streamed_video_format.width=video_width;
    persist();
    return true;
  }
  bool set_video_height(int video_height){
    if(!openhd::validate_video_height(video_height)){
      return false;
    }
    unsafe_get_settings().streamed_video_format.height=video_height;
    persist();
    return true;
  }
  bool set_video_fps(int fps){
    if(!openhd::validate_video_fps(fps)){
      return false;
    }
    unsafe_get_settings().streamed_video_format.framerate=fps;
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
  //
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
    if(m_camera.type==CameraType::RaspberryPiVEYE){
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
};

static std::shared_ptr<CameraHolder> createDummyCamera2(){
  return std::make_shared<CameraHolder>(createDummyCamera());
}

#endif
