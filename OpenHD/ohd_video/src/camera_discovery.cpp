#include "camera_discovery.h"

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <iostream>
#include <regex>

#include "camera.hpp"
#include "camera_discovery_helper.hpp"
#include "libcamera_detect.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"

// annoying linux platform specifics
#ifndef V4L2_PIX_FMT_H265
#define V4L2_PIX_FMT_H265 V4L2_PIX_FMT_HEVC
#endif

std::vector<Camera> DCameras::discover(const OHDPlatform platform) {
  auto m_console=openhd::log::create_or_get("v_dcameras");
  assert(m_console);
  auto m_enable_debug=OHDUtil::get_ohd_env_variable_bool("OHD_DISCOVER_CAMERAS_DEBUG");
  // always enabled for now
  // TODO fixme
  if(m_enable_debug){
    m_console->set_level(spd::level::debug);
    m_console->debug("m_enable_debug=true");
  }
  m_console->debug("discover_internal()");
  std::vector<Camera> cameras;
  // Only on raspberry pi with the old broadcom stack we need a special detection method for the rpi CSI camera.
  // On all other platforms (for example jetson) the CSI camera is exposed as a normal V4l2 linux device,and we cah
  // check the driver if it is actually a CSI camera handled by nvidia.
  // Note: With libcamera, also the rpi will do v4l2 for cameras.
  if(platform.platform_type==PlatformType::RaspberryPi){
    // Detect RPI CSI Camera(s).
    // We can do mmal, libcamera and veye v4l2
    const auto rpi_broadcom_csi_cams=detect_raspberrypi_broadcom_csi(m_console);
    m_console->debug("RPI MMAL CSI Cameras:{}",rpi_broadcom_csi_cams.size());
    OHDUtil::vec_append(cameras,rpi_broadcom_csi_cams);
    const auto rpi_veye_csi_cams= detect_rapsberrypi_veye_v4l2_dirty(m_console);
    m_console->debug("RPI Veye V4l2 CSI Cameras:{}",rpi_veye_csi_cams.size());
    OHDUtil::vec_append(cameras,rpi_veye_csi_cams);
    if(rpi_veye_csi_cams.empty()){
      // NOTE: See the log below why we only detect "libcamera camera(s)" if there are no veye cameras found.
      const auto rpi_libcamera_csi_cams=detect_raspberrypi_libcamera_csi(m_console);
      OHDUtil::vec_append(cameras,rpi_libcamera_csi_cams);
    }else{
      m_console->warn("Skipping libcamera detect, since it might pick up a veye cam by accident even though it cannot do it");
    }
  }else if(platform.platform_type == PlatformType::Allwinner){
    auto tmp=detect_allwinner_csi(m_console);
    OHDUtil::vec_append(cameras,tmp);
  }else if(platform.platform_type == PlatformType::Jetson){
    auto tmp=detect_jetson_csi(m_console);
    OHDUtil::vec_append(cameras,tmp);
  }
  // Allwinner 3.4 kernel v4l2 implementation is so sketchy that probing it can stop it working.
  if(platform.platform_type != PlatformType::Allwinner){
    // I think these need to be run before the detectv4l2 ones, since they are then picked up just like a normal v4l2 camera ??!!
    // Will need custom debugging before anything here is usable again though.
    DThermalCamerasHelper::enableFlirIfFound();
    DThermalCamerasHelper::enableSeekIfFound();
    // NOTE: be carefully to not detect camera(s) twice
    auto usb_cameras= detect_usb_cameras(platform,m_console);
    m_console->debug("N USB Camera(s): {}",usb_cameras.size());
    OHDUtil::vec_append(cameras,usb_cameras);
  }
  for(int i=0;i<cameras.size();i++){
    cameras[i].index=i;
  }
  // write to json for debugging
  write_camera_manifest(cameras);
  return cameras;
}

std::vector<Camera> DCameras::detect_raspberrypi_broadcom_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_raspberrypi_broadcom_csi()");
  std::vector<Camera> ret;
  const auto vcgencmd_result=OHDUtil::run_command_out("vcgencmd get_camera");
  if(vcgencmd_result==std::nullopt){
    m_console->debug("detect_raspberrypi_broadcom_csi() vcgencmd not found");
    return {};
  }
  const auto& raw_value=vcgencmd_result.value();
  std::smatch result;
  // example "supported=2 detected=2"
  const std::regex r{R"(supported=([\d]+)\s+detected=([\d]+))"};
  if (!std::regex_search(raw_value, result, r)) {
    m_console->debug("detect_raspberrypi_broadcom_csi() no regex match");
    return {};
  }
  if (result.size() != 3) {
    m_console->debug("detect_raspberrypi_broadcom_csi() regex unexpected result");
    return {};
  }
  const std::string supported = result[1];
  const std::string detected = result[2];
  m_console->debug("detect_raspberrypi_broadcom_csi() supported={} detected={}",supported,detected);
  const auto camera_count=OHDUtil::string_to_int(detected);
  if (camera_count >= 1) {
    Camera camera;
    camera.name = "Pi_CSI_0";
    camera.vendor = "RaspberryPi";
    camera.type = CameraType::RPI_CSI_MMAL;
    camera.bus = "0";
    ret.push_back(camera);
  }
  if (camera_count >= 2) {
    Camera camera;
    camera.name = "Pi_CSI_1";
    camera.vendor = "RaspberryPi";
    camera.type = CameraType::RPI_CSI_MMAL;
    camera.bus = "1";
    ret.push_back(camera);
  }
  return ret;
}

std::vector<Camera> DCameras::detect_allwinner_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_allwinner_csi(");
  if(OHDFilesystemUtil::exists("/dev/video0")){
    m_console->debug("Camera set as Allwinner_CSI_0");
    Camera camera;
    camera.name = "Allwinner_CSI_0";
    camera.vendor = "Allwinner";
    camera.type = CameraType::ALLWINNER_CSI;
    camera.bus = "0";
    return {camera};
  }
  return {};
}

std::vector<Camera> DCameras::detect_rapsberrypi_veye_v4l2_dirty(std::shared_ptr<spdlog::logger>& m_console) {
  const auto v4l2_info_video0_opt=OHDUtil::run_command_out("v4l2-ctl --info --device /dev/video0");
  if(!v4l2_info_video0_opt.has_value()){
    m_console->warn("Veye detetct unexpected result, autodetect doesnt work");
    return {};
  }
  const auto& v4l2_info_video0=v4l2_info_video0_opt.value();
  bool has_veye=OHDUtil::contains(v4l2_info_video0,"veye327") || OHDUtil::contains(v4l2_info_video0,"csimx307");
  if(OHDFilesystemUtil::exists("/boot/tmp_force_veye.txt")){
    m_console->warn("Forcing veye");
    has_veye= true;
  }
  if(!has_veye){
    return {};
  }
  m_console->info("Detected veye CSI camera");
  Camera camera;
  camera.type=CameraType::RPI_VEYE_CSI_V4l2;
  camera.bus="0";
  camera.index=0;
  camera.name = "Pi_VEYE_0";
  camera.vendor = "VEYE";
  return {camera};
}

#ifdef OPENHD_LIBCAMERA_PRESENT
std::vector<Camera> DCameras::detect_raspberrypi_libcamera_csi(std::shared_ptr<spdlog::logger>& m_console) {
  std::vector<Camera> ret;
  m_console->debug("detect_raspberry_libcamera()");
  auto cameras = openhd::libcameradetect::get_csi_cameras();
  m_console->debug("Libcamera:discovered {} cameras",cameras.size());
  for (const auto& camera : cameras) {
    // TODO: filter out other cameras
    ret.push_back(camera);
  }
  return ret;
}
#else
std::vector<Camera> DCameras::detect_raspberrypi_libcamera_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->warn("detect_raspberry_libcamera - built without libcamera, libcamera features unavailable");
  return {};
}
#endif

std::vector<Camera> DCameras::detect_jetson_csi(std::shared_ptr<spdlog::logger> &m_console) {
  const auto devices=openhd::v4l2::findV4l2VideoDevices();
  for(const auto& device:devices){
    auto v4l2_fp_holder=std::make_unique<openhd::v4l2::V4l2FPHolder>(device,PlatformType::Jetson);
    if(!v4l2_fp_holder->opened_successfully()){
      continue;
    }
    const auto caps_opt= get_capabilities(v4l2_fp_holder);
    if(!caps_opt){
      continue;
    }
    const auto caps=caps_opt.value();
    const std::string driver((char *)caps.driver);
    if(driver=="tegra-video"){
      m_console->debug("Found Jetson CSI camera");
      Camera camera;
      camera.type=CameraType::JETSON_CSI;
      camera.bus="0";
      camera.index=0;
      camera.name = "JETSON_CSI_0";
      camera.vendor = "NVIDIA";
      return {camera};
    }
  }
  return {};
}

/**
 * Helper for checking if a v4l2 device can output any of the supported endpoint format(s).
 * Returns std::nullopt if this device cannot do h264,h265,mjpeg or RAW out.
 */
struct XValidEndpoint{
  v4l2_capability caps;
  openhd::v4l2::EndpointFormats formats;
};
static std::optional<XValidEndpoint> probe_v4l2_device(const OHDPlatform platform,std::shared_ptr<spdlog::logger>& m_console,const std::string& device_node){
  auto v4l2_fp_holder=std::make_unique<openhd::v4l2::V4l2FPHolder>(device_node,platform.platform_type);
  if(!v4l2_fp_holder->opened_successfully()){
    m_console->debug("Can't open {}",device_node);
    return std::nullopt;
  }
  const auto caps_opt=openhd::v4l2::get_capabilities(v4l2_fp_holder);
  if(!caps_opt){
    m_console->debug("Can't get caps for {}",device_node);
    return std::nullopt;
  }
  const auto caps=caps_opt.value();
  const auto supported_formats=openhd::v4l2::iterate_supported_outputs(v4l2_fp_holder);
  if(supported_formats.has_any_valid_format){
    return XValidEndpoint{caps,supported_formats};
  }
  return std::nullopt;
}

std::vector<Camera> DCameras::detect_usb_cameras(const OHDPlatform& platform,std::shared_ptr<spdlog::logger>& m_console) {
  std::vector<Camera> ret{};
  const auto devices = openhd::v4l2::findV4l2VideoDevices();
  for (const auto &device: devices) {
    const auto probed_opt= probe_v4l2_device(platform,m_console,device);
    if(!probed_opt.has_value()){
      continue;
    }
    const auto& probed=probed_opt.value();
    const std::string bus((char *)probed.caps.bus_info);
    const std::string driver((char *)probed.caps.driver);
    CameraType camera_type=CameraType::UNKNOWN;
    if(driver=="uvcvideo"){
      camera_type=CameraType::UVC;
    }else if (driver == "v4l2 loopback") {
      m_console->warn("V4l2-loopback - skipping,");
      continue;
    }else{
      m_console->debug("Unknown driver type {}",driver);
      continue;
    }
    CameraEndpointV4l2 endpoint;
    endpoint.v4l2_device_node=device;
    endpoint.bus=bus;
    endpoint.formats_h264=probed.formats.formats_h264;
    endpoint.formats_h265=probed.formats.formats_h265;
    endpoint.formats_mjpeg=probed.formats.formats_mjpeg;
    endpoint.formats_raw=probed.formats.formats_raw;
    // Find either an already existing cam with this bus or create a new one
    bool found=false;
    for(auto& camera:ret){
      if(camera.bus==bus){
        found= true;
        camera.v4l2_endpoints.push_back(endpoint);
        m_console->debug("Adding endpoint {} to already existing camera",endpoint.v4l2_device_node);
      }
    }
    if(!found){
      Camera camera{};
      camera.type=camera_type;
      camera.bus=endpoint.bus;
      const auto udevadm_info=openhd::v4l2::get_udev_adm_info(device,m_console);
      camera.name=udevadm_info.id_model;
      camera.vendor=udevadm_info.id_vendor;
      camera.v4l2_endpoints.push_back(endpoint);
      m_console->debug("Adding new camera {}:{} for endpoint {}", camera_type_to_string(camera_type),camera.name,endpoint.v4l2_device_node);
      ret.push_back(camera);
    }
  }
  return ret;
}
