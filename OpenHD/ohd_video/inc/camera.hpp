//
// Created by consti10 on 17.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_

#include "camera_enums.hpp"
#include "include_json.hpp"


// Information about a discovered camera and its capabilities.
// NOTE: This does not include any (persistent) settings ! Immutable data (e.g. the discovered camera
// and its capabilities) is seperated from the camera settings (see camera_settings.hpp)

// NOTE: CameraEndpoint is only used for USB cameras and more that use the V4l2 interface. For anything else
// (E.g. CSI Camera(s)) this separation is of no use, since generally there is a streaming pipeline for the different
// formats each and we use the info which platform we are running on / which camera is connected to figure out
// supported formats.
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
  // All the endpoints supported by this camera, can be unused (e.g. for CSI cameras)
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
  // supported by pretty much any camera type (not supporting bitrate control is only the case for these exotic cases)
  [[nodiscard]] bool supports_bitrate()const{
    const bool not_supported= type==CameraType::CustomUnmanagedCamera || type==CameraType::IP;
    return !not_supported;
  }
  // also, pretty much a must have unless using ip camera
  [[nodiscard]] bool supports_changing_format()const{
    const bool not_supported= type==CameraType::CustomUnmanagedCamera || type==CameraType::IP;
    return !not_supported;
  }
  [[nodiscard]] bool supports_keyframe_interval()const{
    const bool not_supported= type==CameraType::CustomUnmanagedCamera || type==CameraType::IP;
    return !not_supported;
  }
  [[nodiscard]] bool supports_rotation()const{
    return type==CameraType::RaspberryPiCSI;
  }
  [[nodiscard]] bool supports_hflip_vflip()const{
    return type==CameraType::RaspberryPiCSI;
  }
  [[nodiscard]] bool supports_awb()const{
    return type==CameraType::RaspberryPiCSI;
  }
  [[nodiscard]] bool supports_exp()const{
    return type==CameraType::RaspberryPiCSI;
  }
  bool supports_brightness()const{
    return type==CameraType::RaspberryPiCSI;
  }
  bool supports_iso()const{
    return type==CameraType::RaspberryPiCSI;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,sensor_name,vid,pid,bus,index,endpoints)

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

static Camera createCustomUnmanagedCamera(){
  Camera camera;
  camera.name = "CustomUnmanagedCamera";
  camera.index = 0;
  camera.vendor = "unknown";
  camera.type = CameraType::CustomUnmanagedCamera;
  return camera;
}

static Camera createCustomIpCamera(){
  Camera camera;
  camera.name = "CustomIpCamera";
  camera.index = 0;
  camera.vendor = "unknown";
  camera.type = CameraType::IP;
  return camera;
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
