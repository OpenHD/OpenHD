#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "openhd-camera-enums.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-settings.hpp"
#include "openhd-settings2.hpp"

#include "mavlink_settings/ISettingsComponent.h"
#include "v_validate_settings.h"

static constexpr auto DEFAULT_BITRATE_KBITS = 5000;

// Return true if the bitrate is considered sane, false otherwise
static bool check_bitrate_sane(const int bitrateKBits) {
  if (bitrateKBits <= 100 || bitrateKBits > (1024 * 1024 * 50)) {
    return false;
  }
  return true;
}

// User-selectable camera options
// These values are settings that can change dynamically at run time
// (non-deterministic)
struct CameraSettings {
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
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar
  // form
  std::string url;
  // enable/disable recording to file
  Recording air_recording=Recording::DISABLED;
  // todo they are simple for the most part, but rn not implemented yet.
  /*std::string brightness;
  std::string contrast;
  std::string sharpness;
  std::string rotate;
  std::string wdr;
  std::string denoise;
  std::string thermal_palette;
  std::string thermal_span;*/
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings,userSelectedVideoFormat,bitrateKBits,url,air_recording)

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
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,vid,pid,bus,index,endpoints)


static const std::string VIDEO_SETTINGS_DIRECTORY=std::string(BASE_PATH)+std::string("video/");

// Camera Holder is used to
// 1) Differentiate between immutable information (camera) and
// 2) mutable camera settings.
// Changes in the camera settings are propagated through this class.
class CameraHolder:public openhd::settings::PersistentSettings<CameraSettings>,
	    public openhd::ISettingsComponent{
 public:
  explicit CameraHolder(Camera camera):
	  _camera(std::move(camera)),
	  openhd::settings::PersistentSettings<CameraSettings>(VIDEO_SETTINGS_DIRECTORY){
	init();
  }
  [[nodiscard]] const Camera& get_camera()const{
	return _camera;
  }
  // Settings hacky begin
  std::vector<openhd::Setting> get_all_settings() override{
	auto c_width=[this](std::string,int value) {
	  return set_video_width(value);
	};
	auto c_height=[this](std::string,int value) {
	  return set_video_height(value);
	};
	auto c_fps=[this](std::string,int value) {
	  return set_video_fps(value);
	};
	auto c_format=[this](std::string,int value) {
	  return set_video_format(value);
	};
	auto c_bitrate=[this](std::string,int value) {
	  return set_video_bitrate(value);
	};
	auto c_recording=[this](std::string,int value) {
	  return set_air_recording(value);
	};
	std::vector<openhd::Setting> ret={
		openhd::Setting{"VIDEO_WIDTH",openhd::IntSetting{get_settings().userSelectedVideoFormat.width,c_width}},
		openhd::Setting{"VIDEO_HEIGHT",openhd::IntSetting{get_settings().userSelectedVideoFormat.height,c_height}},
		openhd::Setting{"VIDEO_FPS",openhd::IntSetting{get_settings().userSelectedVideoFormat.framerate,c_fps}},
		openhd::Setting{"VIDEO_FORMAT",openhd::IntSetting{video_codec_to_int(get_settings().userSelectedVideoFormat.videoCodec),c_format}},
		openhd::Setting{"V_BITRATE_MBITS",openhd::IntSetting{static_cast<int>(get_settings().bitrateKBits / 1000),c_bitrate}},
		openhd::Setting{"V_AIR_RECORDING",openhd::IntSetting{recording_to_int(get_settings().air_recording),c_recording}},
	};
	return ret;
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
  bool set_video_format(int format){
	if(!openhd::validate_video_format(format)){
	  return false;
	}
	unsafe_get_settings().userSelectedVideoFormat.videoCodec=video_codec_from_int(format);
	persist();
	return true;
  }
  bool set_video_bitrate(int bitrate_mbits){
	if(!openhd::validate_bitrate_mbits(bitrate_mbits)){
	  return false;
	}
	unsafe_get_settings().bitrateKBits=bitrate_mbits*1000;
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
  // Settings hacky end
 private:
  // Camera info is immutable
  const Camera _camera;
 private:
  [[nodiscard]] std::string get_unique_filename()const override{
	std::stringstream ss;
	ss<<(static_cast<int>(_camera.index))<<"_"<<camera_type_to_string(_camera.type)<<"_"<<_camera.name;
	return ss.str();
  }
  [[nodiscard]] CameraSettings create_default()const override{
	return CameraSettings{};
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
