#include "DCameras.h"
#include "openhd-camera.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"
#include "DCamerasHelper.hpp"
#include "veye-helper.hpp"
#include "libcamera_provider.hpp"

#include <linux/videodev2.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <regex>

DCameras::DCameras(const OHDPlatform ohdPlatform) :
	ohdPlatform(ohdPlatform){
  m_console=openhd::log::create_or_get("ohd_video_dcameras");
  assert(m_console);
  m_enable_debug=OHDUtil::get_ohd_env_variable_bool("OHD_DISCOVER_CAMERAS_DEBUG");
  // always enabled for now
  // TODO fixme
  if(m_enable_debug){
    m_console->set_level(spd::level::debug);
    m_console->debug("DCameras::m_enable_debug=true");
  }
}

DiscoveredCameraList DCameras::discover_internal() {
  m_console->debug("DCameras::discover_internal()");
  // Only on raspberry pi with the old broadcom stack we need a special detection method for the rpi CSI camera.
  // On all other platforms (for example jetson) the CSI camera is exposed as a normal V4l2 linux device,and we cah
  // check the driver if it is actually a CSI camera handled by nvidia.
  // Note: With libcamera, also the rpi will do v4l2 for cameras.
  if(ohdPlatform.platform_type==PlatformType::RaspberryPi){
    // We need to detect the veye camera first - since once a veye camera is detected and
    // ? we either run the veye_raspivid or do the initializing stuff ? the "normal" rpi camera detection
    // hangs infinite.
    if(detect_raspberrypi_broadcom_veye()){
      m_console->warn("WARNING detected veye camera, skipping normal rpi camera detection");
    }else{
      detect_raspberrypi_broadcom_csi();
    }
  }
  // I think these need to be run before the detectv4l2 ones, since they are then picked up just like a normal v4l2 camera ??!!
  // Will need custom debugging before anything here is usable again though.
  DThermalCamerasHelper::enableFlirIfFound();
  DThermalCamerasHelper::enableSeekIfFound();
  // This will detect all cameras (CSI or not) that do it the proper way (linux v4l2)
  detect_v4l2();
  detect_ip();
  if (ohdPlatform.platform_type == PlatformType::RaspberryPi) {
    detect_raspberry_libcamera();
  }
  argh_cleanup();
  // write to json for debugging
  write_camera_manifest(m_cameras);
  return m_cameras;
}

void DCameras::detect_raspberrypi_broadcom_csi() {
  m_console->debug("DCameras::detect_raspberrypi_broadcom_csi()");
  const auto vcgencmd_result=OHDUtil::run_command_out("vcgencmd get_camera");
  if(vcgencmd_result==std::nullopt){
	m_console->debug("DCameras::detect_raspberrypi_broadcom_csi() vcgencmd not found");
	return;
  }
  const auto& raw_value=vcgencmd_result.value();
  std::smatch result;
  // example "supported=2 detected=2"
  const std::regex r{R"(supported=([\d]+)\s+detected=([\d]+))"};
  if (!std::regex_search(raw_value, result, r)) {
	m_console->debug("DCameras::detect_raspberrypi_broadcom_csi() no regex match");
	return;
  }
  if (result.size() != 3) {
	m_console->debug("DCameras::detect_raspberrypi_broadcom_csi() regex unexpected result");
	return;
  }
  const std::string supported = result[1];
  const std::string detected = result[2];
  m_console->debug("DCameras::detect_raspberrypi_broadcom_csi() supported=" + supported + " detected=" + detected);
  const auto camera_count = atoi(detected.c_str());
  if (camera_count >= 1) {
	Camera camera;
	camera.name = "Pi_CSI_0";
	camera.vendor = "RaspberryPi";
	camera.type = CameraType::RaspberryPiCSI;
	camera.bus = "0";
	camera.index = m_discover_index;
	m_discover_index++;
	CameraEndpoint endpoint=DRPICamerasHelper::createCameraEndpointRpi(false);
	m_camera_endpoints.push_back(endpoint);
	m_cameras.push_back(camera);
  }
  if (camera_count >= 2) {
	Camera camera;
	camera.name = "Pi_CSI_1";
	camera.vendor = "RaspberryPi";
	camera.type = CameraType::RaspberryPiCSI;
	camera.bus = "1";
	camera.index = m_discover_index;
	m_discover_index++;
	CameraEndpoint endpoint=DRPICamerasHelper::createCameraEndpointRpi(true);
	m_camera_endpoints.push_back(endpoint);
	m_cameras.push_back(camera);
  }
}

bool DCameras::detect_raspberrypi_broadcom_veye() {
  m_console->debug("DCameras::detect_raspberrypi_broadcom_veye()");
  // First, we use i2cdetect to find out if there is a veye camera
  const auto i2cdetect_veye_result_opt=OHDUtil::run_command_out("i2cdetect -y 10 0x3b 0x3b | grep  '3b'");
  if(!i2cdetect_veye_result_opt.has_value()){
	m_console->debug("i2cdetect run command failed, is it installed ?");
	return false;
  }
  const auto& i2cdetect_veye_result=i2cdetect_veye_result_opt.value();
  m_console->debug("i2cdetect_veye_result:["+i2cdetect_veye_result+"]");
  std::smatch result;
  std::regex r{ "30:                                  3b            "};
  if (!std::regex_search(i2cdetect_veye_result, result, r)) {
        m_console->debug("DCameras::detect_raspberrypi_broadcom_veye() no regex match ");
	return false;
  }
  m_console->debug("Found veye camera");
  // In case there are some veye instance(s) still running from previous OHD run(s)
  openhd::veye::kill_all_running_veye_instances();
  // R.n we are not sure if this script is needed, but for now, leave it in.
  // This script always fails, but works anyways ?
  const auto success=OHDUtil::run_command("/usr/local/share/veye-raspberrypi/camera_i2c_config",{});
  Camera camera;
  camera.name = "Pi_VEYE_0";
  camera.vendor = "VEYE";
  camera.type = CameraType::RaspberryPiVEYE;
  camera.bus = "0";
  camera.index = m_discover_index;
  m_discover_index++;
  CameraEndpoint endpoint{};
  endpoint.bus = "0";
  endpoint.support_h264 = true;
  endpoint.support_mjpeg = false;
  endpoint.formats.emplace_back("H.264|1920x1080@25");
  endpoint.formats.emplace_back("H.264|1920x1080@30");
  m_camera_endpoints.push_back(endpoint);
  m_cameras.push_back(camera);
  return true;
}

#ifdef OPENHD_LIBCAMERA_PRESENT
void DCameras::detect_raspberry_libcamera() {
  m_console->debug("DCameras::detect_raspberry_libcamera()");
  auto cameras = LibcameraProvider::get_cameras();
  m_console->debug("Libcamera:discovered {} cameras",cameras.size());
  for (const auto& camera : cameras) {
    // TODO: filter out other cameras
    m_cameras.push_back(camera);
  }
}
#else
void DCameras::detect_raspberry_libcamera() {
  m_console->info("DCameras::detect_raspberry_libcamera()- no libcamera found at compile time");
}
#endif

void DCameras::detect_v4l2() {
  m_console->debug("DCameras::detect_v4l2()");
  // Get all the devices to take into consideration.
  const auto devices = DV4l2DevicesHelper::findV4l2VideoDevices();
  for (const auto &device: devices) {
	probe_v4l2_device(device);
  }
}

void DCameras::probe_v4l2_device(const std::string &device) {
  m_console->debug("DCameras::probe_v4l2_device()"+device);
  std::stringstream command;
  command << "udevadm info ";
  command << device.c_str();
  const auto udev_info_opt=OHDUtil::run_command_out(command.str().c_str());
  if(udev_info_opt==std::nullopt){
	m_console->debug("udev_info no result");
	return;
  }
  const auto& udev_info=udev_info_opt.value();
  Camera camera;
  // check for device name
  std::smatch model_result;
  const std::regex model_regex{"ID_MODEL=([\\w]+)"};
  if (std::regex_search(udev_info, model_result, model_regex)) {
	if (model_result.size() == 2) {
	  camera.name = model_result[1];
	}
  }
  // check for device vendor
  std::smatch vendor_result;
  const std::regex vendor_regex{"ID_VENDOR=([\\w]+)"};
  if (std::regex_search(udev_info, vendor_result, vendor_regex)) {
	if (vendor_result.size() == 2) {
	  camera.vendor = vendor_result[1];
	}
  }
  // check for vid
  std::smatch vid_result;
  const std::regex vid_regex{"ID_VENDOR_ID=([\\w]+)"};
  if (std::regex_search(udev_info, vid_result, vid_regex)) {
	if (vid_result.size() == 2) {
	  camera.vid = vid_result[1];
	}
  }
  // check for pid
  std::smatch pid_result;
  const std::regex pid_regex{"ID_MODEL_ID=([\\w]+)"};
  if (std::regex_search(udev_info, pid_result, pid_regex)) {
	if (pid_result.size() == 2) {
	  camera.pid = pid_result[1];
	}
  }
  CameraEndpoint endpoint;
  endpoint.device_node = device;
  if (!process_v4l2_node(device, camera, endpoint)) {
	return;
  }
  bool found = false;
  for (auto &stored_camera: m_cameras) {
	if (stored_camera.bus == camera.bus) {
	  found = true;
	}
  }
  if (!found) {
	camera.index = m_discover_index;
	m_discover_index++;
	m_cameras.push_back(camera);
  }
  m_camera_endpoints.push_back(endpoint);
}

bool DCameras::process_v4l2_node(const std::string &node, Camera &camera, CameraEndpoint &endpoint) {
  m_console->debug( "DCameras::process_v4l2_node("+node+")");
  // fucking hell, on jetson v4l2_open seems to be bugged
  // https://forums.developer.nvidia.com/t/v4l2-open-create-core-with-jetpack-4-5-or-later/170624/6
  int fd;
  if(ohdPlatform.platform_type==PlatformType::Jetson){
	fd = open(node.c_str(), O_RDWR | O_NONBLOCK, 0);
  }else{
	fd = v4l2_open(node.c_str(), O_RDWR);
  }
  if (fd == -1) {
	m_console->debug("Can't open: "+node);
	return false;
  }
  struct v4l2_capability caps = {};
  if (ioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) {
	m_console->debug("Capability query failed: "+node);
	return false;
  }
  const std::string driver((char *)caps.driver);
  m_console->debug("Driver is:"+driver);
  if (driver == "uvcvideo") {
	camera.type = CameraType::UVC;
        m_console->debug("Found UVC camera");
  } else if (driver == "tegra-video") {
	camera.type = CameraType::JetsonCSI;
        m_console->debug("Found Jetson CSI camera");
  } else if (driver == "v4l2 loopback") {
	// this is temporary, we are not going to use v4l2loopback for thermal cameras they'll be directly
	// handled by the camera service instead work anyways
	// Consti10: Removed for this release, won't
	//camera.type = CameraTypeV4L2Loopback;
        m_console->debug("Found v4l2 loopback camera (likely a thermal camera), TODO implement me");
	return false;
  } else {
	/*
	 * This is primarily going to be the bcm2835-v4l2 interface on the Pi, and non-camera interfaces.
	 *
	 * We don't want to use the v4l2 interface to the CSI hardware on Raspberry Pi yet, it offers no
	 * advantage over the mmal interface and doesn't offer the same image controls. Once libcamera is
	 * being widely used this will be the way to support those cameras, but not yet.
	 */
        m_console->debug("Found V4l2 device with unknown driver:"+driver);
	return false;
  }
  const std::string bus((char *)caps.bus_info);
  camera.bus = bus;
  endpoint.bus = bus;
  if (!(caps.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
	m_console->debug("Not a capture device: "+node);
	return false;
  }
  struct v4l2_fmtdesc fmtdesc{};
  memset(&fmtdesc, 0, sizeof(fmtdesc));
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
	struct v4l2_frmsizeenum frmsize{};
	frmsize.pixel_format = fmtdesc.pixelformat;
	frmsize.index = 0;
	while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
	  struct v4l2_frmivalenum frmival{};
	  if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
		frmival.index = 0;
		frmival.pixel_format = fmtdesc.pixelformat;
		frmival.width = frmsize.discrete.width;
		frmival.height = frmsize.discrete.height;

		while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
		  if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
			std::stringstream new_format;
			if (fmtdesc.pixelformat == V4L2_PIX_FMT_H264) {
			  endpoint.support_h264 = true;
			}
#if defined V4L2_PIX_FMT_H265
			  else if (fmtdesc.pixelformat == V4L2_PIX_FMT_H265) {
				  endpoint.support_h265 = true;
			  }
#endif
			else if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG) {
			  endpoint.support_mjpeg = true;
			} else {
			  // if it supports something else it's one of the raw formats, the camera service will
			  // figure out what to do with it
			  endpoint.support_raw = true;
			}
			new_format << fmtdesc.description;
			new_format << "|";
			new_format << frmsize.discrete.width;
			new_format << "x";
			new_format << frmsize.discrete.height;
			new_format << "@";
			new_format << frmival.discrete.denominator;
			endpoint.formats.push_back(new_format.str());
                        m_console->debug( "Found format: "+new_format.str());
		  }
		  frmival.index++;
		}
	  }
	  frmsize.index++;
	}
	fmtdesc.index++;
  }
  v4l2_close(fd);
  m_console->debug("DCameras::process_v4l2_node done");
  return true;
}

void DCameras::detect_ip() {
  m_console->debug("Detect_ip unimplemented");
  // Note: I don't think there is an easy way to detect ip cameras,it probably requires some manual user input.
}

void DCameras::argh_cleanup() {
  // Fixup endpoints, would be better to seperate the discovery steps properly so that this is not needed
  for (auto &camera: m_cameras) {
	std::vector<CameraEndpoint> endpointsForThisCamera;
	for (const auto &endpoint: m_camera_endpoints) {
	  if (camera.bus == endpoint.bus) {
		// an endpoint who cannot do anything is just a waste and added complexity for later modules
		if (endpoint.formats.empty()) {
		  // not really an error, since it is an implementation issue during detection that is negated here
		  m_console->debug("Discarding endpoint"+endpoint.device_node+" due to no formats");
		  continue;
		}
		if (!endpoint.supports_anything()) {
		  // not really an error, since it is an implementation issue during detection that is negated here
		  m_console->debug("Discarding endpoint "+endpoint.device_node+" due to no capabilities");
		  continue;
		}
		endpointsForThisCamera.push_back(endpoint);
	  }
	}
	camera.endpoints = endpointsForThisCamera;
	// also, a camera without a endpoint - what the heck should that be
	if (camera.endpoints.empty()) {
	  m_console->warn("Warning Camera without endpoints");
	}
  }
  // make sure the camera indices are right
  int camIdx = 0;
  for (auto &camera: m_cameras) {
	camera.index = camIdx;
	camIdx++;
  }
  write_camera_manifest(m_cameras);
}

DiscoveredCameraList DCameras::discover(const OHDPlatform ohdPlatform) {
  auto discover=DCameras{ohdPlatform};
  return discover.discover_internal();
}

std::vector<std::shared_ptr<CameraHolder>> DCameras::discover2(const OHDPlatform ohdPlatform) {
  auto discovered_cameras= discover(ohdPlatform);
  std::vector<std::shared_ptr<CameraHolder>> ret;
  for(const auto& camera:discovered_cameras){
    ret.emplace_back(std::make_unique<CameraHolder>(camera));
  }
  return ret;
}
