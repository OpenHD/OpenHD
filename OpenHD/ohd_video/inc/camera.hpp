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
struct CameraEndpointV4l2 {
  std::string v4l2_device_node;
  std::string bus;
  std::vector<EndpointFormat> formats_h264;
  std::vector<EndpointFormat> formats_h265;
  std::vector<EndpointFormat> formats_mjpeg;
  std::vector<EndpointFormat> formats_raw;
  bool supports_h264()const{
    return !formats_h264.empty();
  }
  bool supports_h265()const{
    return !formats_h265.empty();
  }
  bool supports_mjpeg()const{
    return !formats_h264.empty();
  }
  bool supports_raw()const{
    return !formats_raw.empty();
  }
  bool supports_anything()const{
    return supports_h264() || supports_h265() || supports_mjpeg() || supports_raw();
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraEndpointV4l2, v4l2_device_node,bus,formats_h264,formats_h265,formats_mjpeg,formats_raw)

/**
 * Note: A camera might output multiple pixel formats / already encoded video formats (and they might even show up as multiple v4l2 device node(s)
 * But discovery should only create one "Camera" instance per connected camera (for example, a USB camera is one Camera even though
 * it might support h264 and raw output, or a CSI camera is one camera even though it might support raw bayer on one node, and a then a ISP
 * processed format (e.g. YUV) on another node).
 */
struct Camera {
  CameraType type = CameraType::UNKNOWN;
  std::string name = "unknown";
  std::string vendor = "unknown";
  std::string sensor_name="unknown";
  // for USB this is the bus number, for CSI it's the connector number
  std::string bus;
  // Unique index of this camera, should start at 0. The index number depends on
  // the order the cameras were picked up during the discovery step.
  int index = 0;
  // All the endpoints (aka supported video resolution, framerate and pixel format
  // NOTE: R.n we only use this for V4l2 UVC camera(s) aka usb cameras, since for the CSI camera(s)
  // we have no resolution / framerate checking (it is just too complicated to both take the CSI camera caps,
  // and the platform encode cap(s) into account).
  std::vector<CameraEndpointV4l2> v4l2_endpoints;
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
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP;
    return !not_supported;
  }
  // also, pretty much a must have unless using ip camera
  [[nodiscard]] bool supports_changing_format()const{
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP || type==CameraType::RPI_VEYE_CSI_V4l2;
    return !not_supported;
  }
  [[nodiscard]] bool supports_keyframe_interval()const{
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP;
    return !not_supported;
  }
  [[nodiscard]] bool supports_rotation()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  [[nodiscard]] bool supports_hflip_vflip()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  [[nodiscard]] bool supports_awb()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  [[nodiscard]] bool supports_exp()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_brightness()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_iso()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_rpi_rpicamsrc_metering_mode()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,sensor_name,bus,index, v4l2_endpoints)

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
  camera.type = CameraType::DUMMY_SW;
  return camera;
}

static Camera createCustomUnmanagedCamera(){
  Camera camera;
  camera.name = "CustomUnmanagedCamera";
  camera.index = 0;
  camera.vendor = "unknown";
  camera.type = CameraType::CUSTOM_UNMANAGED_CAMERA;
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

// Returns the first endpoint found that can output the given video codec (aka non-raw)
static std::optional<CameraEndpointV4l2> get_endpoint_supporting_codec(const std::vector<CameraEndpointV4l2>& endpoints,const VideoCodec codec){
  for (const auto &endpoint: endpoints) {
    if ( codec == VideoCodec::H264 && endpoint.supports_h264()) {
      return endpoint;
    }
    if ( codec == VideoCodec::H265 && endpoint.supports_h265()) {
      return endpoint;
    }
    if ( codec == VideoCodec::MJPEG && endpoint.supports_mjpeg()) {
      return endpoint;
    }
  }
  return std::nullopt;
}

// Returns the first endpoint found that can output any "RAW" format, we do not differentiate between RAW format(s) since we can
// always convert it via gstreamer. Note - raw in this context means already processed by ISP, aka RGB, YUV, ... - not BAYER or something
// completely unusable
static std::optional<CameraEndpointV4l2> get_endpoint_supporting_raw(const std::vector<CameraEndpointV4l2>& endpoints){
  for (const auto &endpoint: endpoints) {
    if(endpoint.supports_raw()){
      return endpoint;
    }
  }
  return std::nullopt;
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
