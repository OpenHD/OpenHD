//
// Created by consti10 on 17.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_

#include "include_json.hpp"

// Information about a discovered camera and its capabilities.
// This does not include any (persistent) settings !

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

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
