#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "openhd-camera-enums.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-settings.hpp"
#include "openhd-settings2.hpp"

#include "mavlink_settings/ISettingsComponent.hpp"
#include "v_validate_settings.h"
#include "openhd-action-handler.hpp"

static constexpr int DEFAULT_BITRATE_KBITS = 5000;
static constexpr int DEFAULT_KEYFRAME_INTERVAL = 30;
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
  // and the user wants to fly home without video, using only telemetry / HUD
  bool enable_streaming=true;
  // The video format selected by the user. If the user sets a video format that
  // isn't supported (for example, he might select h264|1920x1080@120 but the
  // camera can only do 60fps) The stream should default to the first available
  // video format. If no video format is available, it should default to
  // h264|640x480@30.
  VideoFormat userSelectedVideoFormat{VideoCodec::H264, 640, 480, 30};
  // All these are for the future, and probably implemented on a best effort
  // approach- e.g. changing them does not neccessarly mean the camera supports
  // changing them, and they are too many to do it in a "check if supported"
  // manner.
  //--
  // The bitrate the generated stream should have. Note that not all cameras /
  // encoder support a constant bitrate, and not all encoders support all
  // bitrates, especially really low ones.
  int bitrateKBits = DEFAULT_BITRATE_KBITS;
  // r.n use rpicamrs as reference. Not supported by all cameras
  int keyframe_interval=DEFAULT_KEYFRAME_INTERVAL;
  // MJPEG has no bitrate parameter, only a "quality" param. This value is only used if the
  // user selected MJPEG as its video codec
  int mjpeg_quality_percent=DEFAULT_MJPEG_QUALITY_PERCENT;
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar
  std::string url;
  // enable/disable recording to file
  Recording air_recording=Recording::DISABLED;

  // only used on RK3588
  VideoFormat recordingFormat{VideoCodec::H264, 0, 0, 0}; // 0 means copy
  int recordingKBits = DEFAULT_RECORDING_KBITS;
  int recordingQP = DEFAULT_RECORDING_QP;
  RateControlMode recordingRCMode = DEFAULT_RC_MODE;

  // todo they are simple for the most part, but rn not implemented yet.
  // camera rotation, allowed values:
  // 0 nothing
  // 90° to the right
  // 180° to the right
  // 270° to the right
  // Note that r.n only rpi camera supports rotation(s), where the degrees are mapped to the corresponding h/v flip(s)
  int camera_rotation_degree=0;
  // R.n only for rpi camera, see https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c
  int awb_mode=1; //default 1 (auto)
  int exposure_mode=1; //default 1 (auto)
  // see gst-rpicamsrc documentation
  // NOTE: We do not like negative values in OpenHD, so we use a different mapping here than the enum from gst-rpicamsrc
  // default to 0 (none)
  int intra_refresh_type=0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings,enable_streaming,userSelectedVideoFormat,bitrateKBits,keyframe_interval,mjpeg_quality_percent,url,air_recording,camera_rotation_degree,
                                   awb_mode,exposure_mode)

struct CameraEndpoint {
  std::string device_node;
  std::string bus;
  bool support_h264 = false;
  bool support_h265 = false;
  bool support_mjpeg = false;
  bool support_raw = false;
  std::vector<std::string> formats;
  // Consti10: cleanup- an endpoint that supports nothing, what the heck should
  // we do with that ;)
  [[nodiscard]] bool supports_anything() const {
    return (support_h264 || support_h265 || support_mjpeg || support_raw);
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraEndpoint,device_node,bus,support_h264,support_h265,support_mjpeg,support_raw,formats)

struct Camera {
  CameraType type = CameraType::Unknown;
  std::string name = "unknown";
  std::string vendor = "unknown";
  std::string sensor_name="unknown";
  std::string vid;
  std::string pid;
  // for USB this is the bus number, for CSI it's the connector number
  std::string bus;
  // Unique index of this camera, should start at 0. The index number depends on
  // the order the cameras were picked up during the discovery step.
  int index = 0;
  // All the endpoints supported by this camera.
  std::vector<CameraEndpoint> endpoints;
  /**
   * For logging, create a quick name string that gives developers enough info
   * such that they can figure out what this camera is.
   * @return verbose string.
   */
  [[nodiscard]] std::string debugName() const {
    std::stringstream ss;
    ss << name << "|" << camera_type_to_string(type);
    return ss.str();
  }
  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    ss << "Camera" << index << "{" << camera_type_to_string(type) << ""
       << "}";
    return ss.str();
  }
  [[nodiscard]] bool supports_rotation()const{
    return type==CameraType::RaspberryPiCSI || type==CameraType::RaspberryPiVEYE;
  }
  [[nodiscard]] bool supports_awb()const{
    return type==CameraType::RaspberryPiCSI || type==CameraType::RaspberryPiVEYE;
  }
  [[nodiscard]] bool supports_exp()const{
    return type==CameraType::RaspberryPiCSI || type==CameraType::RaspberryPiVEYE;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,sensor_name,vid,pid,bus,index,endpoints)


static const std::string VIDEO_SETTINGS_DIRECTORY=std::string(BASE_PATH)+std::string("video/");

// Camera Holder is used to
// 1) Differentiate between immutable information (camera) and
// 2) mutable camera settings.
// Changes in the camera settings are propagated through this class.
class CameraHolder:public openhd::settings::PersistentSettings<CameraSettings>,
                     public openhd::ISettingsComponent{
 public:
  explicit CameraHolder(Camera camera,std::shared_ptr<openhd::ActionHandler> opt_action_handler= nullptr):
                                                                                                              _camera(std::move(camera)),m_opt_action_handler(std::move(opt_action_handler)),
                                                                                                              openhd::settings::PersistentSettings<CameraSettings>(VIDEO_SETTINGS_DIRECTORY){
    init();
    if(m_opt_action_handler){
      m_opt_action_handler->action_set_video_codec_handle(video_codec_to_int(get_settings().userSelectedVideoFormat.videoCodec));
    }
  }
  [[nodiscard]] const Camera& get_camera()const{
    return _camera;
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
                                        openhd::video_format_from_int_values(get_settings().userSelectedVideoFormat.width,
                                                                             get_settings().userSelectedVideoFormat.height,
                                                                             get_settings().userSelectedVideoFormat.framerate),
                                        c_width_height_framerate
                                    }},
        openhd::Setting{"VIDEO_CODEC",openhd::IntSetting{video_codec_to_int(get_settings().userSelectedVideoFormat.videoCodec), c_codec}},
        openhd::Setting{"V_BITRATE_MBITS",openhd::IntSetting{static_cast<int>(get_settings().bitrateKBits / 1000),c_bitrate}},
        openhd::Setting{"V_KEYFRAME_I",openhd::IntSetting{get_settings().keyframe_interval,c_keyframe_interval}},
        openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{recording_to_int(get_settings().air_recording),c_recording}},
        openhd::Setting{"V_MJPEG_QUALITY",openhd::IntSetting{get_settings().mjpeg_quality_percent,c_mjpeg_quality_percent}},
        // for debugging
        openhd::Setting{"V_CAM_TYPE",openhd::StringSetting { get_short_name(),c_read_only_param}},
    };
    if(_camera.type==CameraType::Libcamera){
      // r.n we only write the sensor name for cameras detected via libcamera
      ret.push_back(openhd::Setting{"V_CAM_SENSOR",openhd::StringSetting{_camera.sensor_name,c_read_only_param}});
    }
    if(_camera.supports_rotation()){
      auto c_rotation=[this](std::string,int value) {
        return set_camera_rotation(value);
      };
      ret.push_back(openhd::Setting{"V_CAM_ROT_DEG",openhd::IntSetting{get_settings().camera_rotation_degree,c_rotation}});
    }
    if(_camera.supports_awb()){
      auto cb=[this](std::string,int value) {
        return set_camera_awb(value);
      };
      ret.push_back(openhd::Setting{"V_AWB_MODE",openhd::IntSetting{get_settings().awb_mode,cb}});
    }
    if(_camera.supports_exp()){
      auto cb=[this](std::string,int value) {
        return set_camera_exposure(value);
      };
      ret.push_back(openhd::Setting{"V_EXP_MODE",openhd::IntSetting{get_settings().exposure_mode,cb}});
    }
    return ret;
  }
  bool set_enable_streaming(int enable){
    if(!(enable==0 || enable==1)){
      return false;
    }
    unsafe_get_settings().enable_streaming=static_cast<bool>(enable);
    persist();
    return true;
  }
  bool set_video_width(int video_width){
    if(!openhd::validate_video_with(video_width)){
      return false;
    }
    unsafe_get_settings().userSelectedVideoFormat.width=video_width;
    persist();
    return true;
  }
  bool set_video_height(int video_height){
    if(!openhd::validate_video_height(video_height)){
      return false;
    }
    unsafe_get_settings().userSelectedVideoFormat.height=video_height;
    persist();
    return true;
  }
  bool set_video_fps(int fps){
    if(!openhd::validate_video_fps(fps)){
      return false;
    }
    unsafe_get_settings().userSelectedVideoFormat.framerate=fps;
    persist();
    return true;
  }
  bool set_video_codec(int codec){
    if(!openhd::validate_video_codec(codec)){
      return false;
    }
    unsafe_get_settings().userSelectedVideoFormat.videoCodec=video_codec_from_int(codec);
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
    unsafe_get_settings().bitrateKBits= mbits_to_kbits_per_second(bitrate_mbits);
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
    if(!_camera.supports_rotation())return false;
    if(!openhd::validate_camera_rotation(value)){
      return false;
    }
    unsafe_get_settings().camera_rotation_degree=value;
    persist();
    return true;
  }
  bool set_keyframe_interval(int value){
    if(!openhd::validate_rpi_keyframe_interval(value))return false;
    unsafe_get_settings().keyframe_interval=value;
    persist();
    return true;
  }
  //
  bool set_camera_awb(int value){
    if(!_camera.supports_awb())return false;
    if(!openhd::validate_rpi_awb_mode(value)){
      return false;
    }
    unsafe_get_settings().awb_mode=value;
    persist();
    return true;
  }
  bool set_camera_exposure(int value){
    if(!_camera.supports_exp())return false;
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
    unsafe_get_settings().userSelectedVideoFormat.width=width;
    unsafe_get_settings().userSelectedVideoFormat.height=height;
    unsafe_get_settings().userSelectedVideoFormat.framerate=framerate;
    persist();
    return true;
  }
  // Settings hacky end
 private:
  // Camera info is immutable
  const Camera _camera;
  std::shared_ptr<openhd::ActionHandler> m_opt_action_handler;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
    std::stringstream ss;
    // TODO: r.n not unique enough, we need to be unique per model,too - e.g. a user
    // might connect a different USB camera, where we'd need a different unique ID for
    ss<<(static_cast<int>(_camera.index))<<"_"<<camera_type_to_string(_camera.type);
    return ss.str();
  }
  [[nodiscard]] CameraSettings create_default()const override{
    auto ret=CameraSettings{};
    if(_camera.type==CameraType::RaspberryPiVEYE){
      // Veye cannot do 640x480@30 by default, this is the next lower possible
      // (TODO it should do 720p but for some reason doesn't)
      ret.userSelectedVideoFormat.width=1920;
      ret.userSelectedVideoFormat.height=1080;
      ret.userSelectedVideoFormat.framerate=30;
    }
    return ret;
  }
  [[nodiscard]] std::string get_short_name()const{
    return camera_type_to_string(_camera.type);
  }
};


using DiscoveredCameraList = std::vector<Camera>;

static nlohmann::json cameras_to_json(const DiscoveredCameraList &cameras) {
  nlohmann::json j;
  for (const auto &camera : cameras) {
    nlohmann::json _camera = camera;
    j.push_back(_camera);
  }
  return j;
}

static constexpr auto CAMERA_MANIFEST_FILENAME = "/tmp/camera_manifest";

static void write_camera_manifest(const DiscoveredCameraList &cameras) {
  auto manifest = cameras_to_json(cameras);
  std::ofstream _t(CAMERA_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

static Camera createDummyCamera() {
  Camera camera;
  camera.name = "DummyCamera";
  camera.index = 0;
  camera.vendor = "dummy";
  camera.type = CameraType::Dummy;
  return camera;
}

static std::shared_ptr<CameraHolder> createDummyCamera2(){
  return std::make_shared<CameraHolder>(createDummyCamera());
}

#endif
