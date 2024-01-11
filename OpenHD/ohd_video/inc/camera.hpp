//
// Created by consti10 on 17.11.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_DISCOVERED_CAMERA_H_

#include "camera_enums.hpp"
#include "openhd_util_filesystem.h"


// Information about a discovered camera and its capabilities.
// NOTE: This does not include any (persistent) settings ! Immutable data (e.g. the discovered camera
// and its capabilities) is seperated from the camera settings (see camera_settings.hpp)

static constexpr int X_CAM_TYPE_DUMMY_SW=0; // Dummy sw picture
static constexpr int X_CAM_TYPE_USB=1; // Any USB camera
static constexpr int X_CAM_TYPE_EXTERNAL=2; // input via udp rtp
static constexpr int X_CAM_TYPE_EXTERNAL_IP=3; // input via udp rtp & file start_ip_cam.txt is created.
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_AUTO=4;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX477M=5;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_SKYMASTERHDR=6;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_SKYVISIONPRO=7;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX477=8;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX462=9;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX327=10;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX290=11;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX462_LOWLIGHT_MINI=12;
static constexpr int X_CAM_TYPE_RPI_VEYE_2MP=13;
static constexpr int X_CAM_TYPE_RPI_VEYE_CSIMX307=14;
static constexpr int X_CAM_TYPE_RPI_VEYE_CSSC132=15;
static constexpr int X_CAM_TYPE_RPI_VEYE_MVCAM=16;
static constexpr int X_CAM_TYPE_X20=17;
static constexpr int X_CAM_TYPE_ROCKCHIP_X0=18; // reserved for future use
static constexpr int X_CAM_TYPE_ROCKCHIP_X1=19;// reserved for future use
static constexpr int X_CAM_TYPE_ROCKCHIP_X2=19;// reserved for future use
// no camera, only exists to have a default value for secondary camera (which is disabled by default).
// NOTE: The primary camera cannot be disabled !
static constexpr int X_CAM_TYPE_DISABLED=2147483647;

struct XCamera {
    int camera_type = X_CAM_TYPE_DUMMY_SW;
    // 0 for primary camera, 1 for secondary camera
    int index;
    // Only valid if camera is of type USB
    std::string usb_v4l2_device_node;
};

static bool rpi_requires_mmal_pipeline(){
    return false;
}
static bool rpi_requires_libcamera_pipeline(){
    return false;
}
static bool rpi_requires_veye_pipeline(){
    return false;
}
static bool is_valid_cam_type(int cam_type){
    return cam_type>=0 && cam_type<=19;
}


/**
 * NOTE:
 * * Even though there is a move towards properly exposing CSI camera(s) via v4l2, this is error prone and
* goes down a rabbit hole quite quickly.
* Aka the proper v4l2 way would be to have each CSI camera exposing it's capabilities, and then also the encoder
* exposing its capabilities. However, there is just no way to do this platform independently - in short, this
* approach is too complex. Period.
*
* This is why we separate camera(s) in different categories and then have (for the most part) the specific gstreamer pipeline(s) for those
* cameras. We only use v4l2 to discover and reason about formats/framerate(s) for "USB Cameras" (aka UVC cameras).
* For USB camera(s), we then seperate by endpoints - since they often provide both an already encoded "pixel format" (aka h264) but also
* raw format(s).
 */

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
    return !formats_mjpeg.empty();
  }
  bool supports_raw()const{
    return !formats_raw.empty();
  }
  bool supports_anything()const{
    return supports_h264() || supports_h265() || supports_mjpeg() || supports_raw();
  }
};

/**
 * Note: A camera might output multiple pixel formats / already encoded video formats (and they might even show up as multiple v4l2 device node(s)
 * But discovery should only create one "Camera" instance per connected camera (for example, a USB camera is one Camera even though
 * it might support h264 and raw output, or a CSI camera is one camera even though it might support raw bayer on one node, and a then a ISP
 * processed format (e.g. YUV) on another node).
 */
struct Camera {
  CameraType type = CameraType::UNKNOWN;
  //These are not mandatory, but quite usefully for keeping track of camera(s).
  std::string name = "unknown";
  std::string vendor = "unknown";
  std::string sensor_name="unknown";
  // for USB this is the bus number, for CSI it's the connector number
  std::string bus;
  // Unique index of this camera, should start at 0. The index number depends on
  // the order the cameras were picked up during the discovery step.
  int index = 0;
  // this is only for camera tye RPI_CSI_MMAL - differentiate between CSI camera and the HDMI to CSI adapter
  // (since the second one needs workarounds)
  bool rpi_csi_mmal_is_csi_to_hdmi=false;
  // All the endpoints (aka supported video resolution, framerate and pixel format
  // NOTE: R.n we only use this for V4l2 UVC camera(s) aka usb cameras, since for the CSI camera(s)
  // we have no resolution / framerate checking (it is just too complicated to both take the CSI camera caps,
  // and the platform encode cap(s) into account).
  std::vector<CameraEndpointV4l2> v4l2_endpoints;
  /**
   * For logging, create a verbose string that gives developers enough info
   * such that they can figure out what exactly this camera is.
   */
  [[nodiscard]] std::string to_long_string() const {
    return fmt::format("{}:{}:{}:{}:{}:{}", camera_type_to_string(type),name,vendor,sensor_name,bus,index);
  }
  [[nodiscard]] std::string to_short_string() const {
    return fmt::format("{}:{}:{}", camera_type_to_string(type),name,index);
  }
  // There are 2 types of bitrate control, the more the camera supports the better:
  // 1) Changing the bitrate with a complete restart of the streaming pipeline (not ideal, but better than nothing)
  // 2) Changing the bitrate at run time without a complete restart of the streaming pipeline
  [[nodiscard]] bool supports_bitrate_with_restart()const{
    if(type==CameraType::DUMMY_SW || type == CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA || type==CameraType::RPI_CSI_VEYE_V4l2
        // NOTE ! USB Camera(s) - generally only supported if we use sw encode with the camera, but we do not know here what is being done
        || type==CameraType::UVC ){
      return true;
    }
    return false;
  }
  [[nodiscard]] bool supports_bitrate_without_restart()const{
    if(type==CameraType::DUMMY_SW || type==CameraType::RPI_CSI_MMAL)return true;
    return false;
  }
  // supported by pretty much any camera type (not supporting bitrate control is only the case for these exotic cases)
  [[nodiscard]] bool supports_bitrate()const{
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP;
    return !not_supported;
  }
  // also, pretty much a must have unless using ip camera
  [[nodiscard]] bool supports_changing_format()const{
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP || type==CameraType::RPI_CSI_VEYE_V4l2;
    return !not_supported;
  }
  [[nodiscard]] bool supports_keyframe_interval()const{
    const bool not_supported= type==CameraType::CUSTOM_UNMANAGED_CAMERA || type==CameraType::IP;
    return !not_supported;
  }
  [[nodiscard]] bool supports_rotation()const{
    return type==CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA; // requires openhd libcamera
  }
  [[nodiscard]] bool supports_hflip_vflip()const{
    return type==CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA; // requires openhd libcamera
  }
  [[nodiscard]] bool supports_awb()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  [[nodiscard]] bool supports_exp()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_brightness()const{
    return type==CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA; // requires openhd libcamera
  }
  bool supports_sharpness()const{
    return type==CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA; // requires openhd libcamera
  }
  bool supports_iso()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_rpi_rpicamsrc_metering_mode()const{
    return type==CameraType::RPI_CSI_MMAL;
  }
  bool supports_force_sw_encode()const{
      return type==CameraType::UVC || type==CameraType::RPI_CSI_MMAL || type==CameraType::RPI_CSI_LIBCAMERA;
  }
  // This "hash" needs to be deterministic and unique - otherwise, incompatible settings from
  // a previous camera might be used
  std::string get_unique_settings_filename()const{
    auto safe_name=name;
    // TODO find a better solution
    if(type==CameraType::RPI_CSI_LIBCAMERA){
      safe_name=sensor_name;
    }
    return fmt::format("{}_{}_{}.json",index, camera_type_to_string(type),safe_name);
  }
};

static Camera createDummyCamera(int index=0) {
  Camera camera;
  camera.name = fmt::format("DummyCamera{}",index);
  camera.index = index;
  camera.vendor = "dummy";
  camera.type = CameraType::DUMMY_SW;
  return camera;
}

static Camera createCustomUnmanagedCamera(int index=0){
  Camera camera;
  camera.name = fmt::format("CustomUnmanagedCamera{}",index);
  camera.index = index;
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
