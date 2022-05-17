//
// Created by consti10 on 16.05.22.
//

#ifndef OPENHD_DCAMERASHELPER_H
#define OPENHD_DCAMERASHELPER_H

#include <libusb.h>
#include <vector>
#include <string>
#include "openhd-util.hpp"

/**
 * Helper for the discover thermal cameras step.
 * It is a bit more complicated, once we actually support them the code here will probably blow a bit.
 * Rn I just copy pasted stephens code for the flir and seek here
 */
namespace DThermalCamerasHelper {
static constexpr auto FLIR_ONE_VENDOR_ID = 0x09cb;
static constexpr auto FLIR_ONE_PRODUCT_ID = 0x1996;

static constexpr auto SEEK_COMPACT_VENDOR_ID = 0x289d;
static constexpr auto SEEK_COMPACT_PRODUCT_ID = 0x0010;

static constexpr auto SEEK_COMPACT_PRO_VENDOR_ID = 0x289d;
static constexpr auto SEEK_COMPACT_PRO_PRODUCT_ID = 0x0011;
/*
 * What this is:
 *
 * We're detecting whether the flir one USB thermal camera is connected. We then run the flir one driver
 * with systemd.
 *
 * What happens after:
 *
 * The systemd service starts, finds the camera and begins running on the device node we select. Then
 * we will let it be found by the rest of this class just like any other camera, so it gets recorded
 * in the manifest and found by the camera service.
 *
 *
 * todo: this should really be marking the camera as a thermal cam instead of starting v4l2loopback and
 *       abstracting it away like this, but the camera service doesn't yet have a thermal handling class
 */
static void enableFlirIfFound() {
  libusb_context *context = nullptr;
  int result = libusb_init(&context);
  if (result) {
	std::cerr << "Failed to initialize libusb" << std::endl;
	return;
  }
  libusb_device_handle *handle = libusb_open_device_with_vid_pid(nullptr, FLIR_ONE_VENDOR_ID, FLIR_ONE_PRODUCT_ID);
  if (handle) {
	std::vector<std::string> ar{
		"start", "flirone"
	};
	OHDUtil::run_command("systemctl", ar);
  }
}

/*
 * What this is:
 *
 * We're detecting whether the 2 known Seek thermal USB cameras are connected, then constructing
 * arguments for the seekthermal driver depending on which model it is. We then run the seek driver
 * with systemd using the arguments file we provided to it in seekthermal.service in the libseek-thermal
 * package.
 *
 * What happens after:
 *
 * The systemd service starts, finds the camera and begins running on the device node we select. Then
 * we will let it be found by the rest of this class just like any other camera, so it gets recorded
 * in the manifest and found by the camera service.
 *
 *
 * todo: this should pull the camera settings from the settings file if available
 */
static void enableSeekIfFound() {
  libusb_context *context = nullptr;
  int result = libusb_init(&context);
  if (result) {
	std::cerr << "Failed to initialize libusb" << std::endl;
	return;
  }

  libusb_device_handle
	  *handle_compact = libusb_open_device_with_vid_pid(nullptr, SEEK_COMPACT_VENDOR_ID, SEEK_COMPACT_PRODUCT_ID);
  libusb_device_handle *handle_compact_pro =
	  libusb_open_device_with_vid_pid(nullptr, SEEK_COMPACT_PRO_VENDOR_ID, SEEK_COMPACT_PRO_PRODUCT_ID);

  // todo: this will need to be pulled from the config, we may end up running these from the camera service so that
  //       it can see the camera settings, which are not visible to openhd-system early at boot
  std::string model;
  std::string fps;

  if (handle_compact) {
	std::cout << "Found seek" << std::endl;
	model = "seek";
	fps = "7";
  }

  if (handle_compact_pro) {
	std::cout << "Found seekpro" << std::endl;
	model = "seekpro";
	// todo: this is not necessarily accurate, not all compact pro models are 15hz
	fps = "15";
  }

  if (handle_compact_pro || handle_compact) {
	std::cout << "Found seek thermal camera" << std::endl;

	std::ofstream _u("/etc/openhd/seekthermal.conf", std::ios::binary | std::ios::out);
	// todo: this should be more dynamic and allow for multiple cameras
	_u << "DeviceNode=/dev/video4";
	_u << std::endl;
	_u << "SeekModel=";
	_u << model;
	_u << std::endl;
	_u << "FPS=";
	_u << fps;
	_u << std::endl;
	_u << "SeekColormap=11";
	_u << std::endl;
	_u << "SeekRotate=11";
	_u << std::endl;
	_u.close();

	std::vector<std::string> ar{
		"start", "seekthermal"
	};
	OHDUtil::run_command("systemctl", ar);
  }
}
}


namespace DRPICamerasHelper{

// For rpi camera, we just have a list of the supported video formats.
// Note that not all CSI cameras can do all of these formats, but at least the default rpi foundation cameras can.
// Stephen:
// these are temporary, there isn't a way to ask the old broadcom camera drivers about the supported
// resolutions, but we know which ones people actually use so we can simply mark them for now.
// NOTE: Rn there is no check on weather the user input is something the pi can do anyways.
static std::vector<std::string> createDefaultSupportedLIstRpiCam(){
  std::vector<std::string> ret;
  ret.emplace_back("H.264|640x480@30");
  ret.emplace_back("H.264|640x480@48");
  ret.emplace_back("H.264|640x480@60");
  ret.emplace_back("H.264|800x480@30");
  ret.emplace_back("H.264|1280x720@30");
  ret.emplace_back("H.264|1280x720@48");
  ret.emplace_back("H.264|1280x720@59.9");
  ret.emplace_back("H.264|1012x760@90");
  // Cannot do ret.emplace_back("H.264|1012x760@120");
  ret.emplace_back("H.264|1920x1080@30");
  // Cannot do ret.emplace_back("H.264|1920x1080@59.9");
  return ret;
}

/**
 * Create a camera endpoint for rpi camera. Rpi has CSI0 and CSI1 for cameras.
 * @param isCsi1 if true, we've detect a CSI1 camera. CSI0 otherwise
 * @return
 */
static CameraEndpoint createCameraEndpointRpi(bool isCsi1= false){
  const auto bus=isCsi1 ? "1":"0";
  CameraEndpoint endpoint;
  endpoint.bus = bus;
  endpoint.support_h264 = true;
  endpoint.support_mjpeg = false;
  endpoint.formats=DRPICamerasHelper::createDefaultSupportedLIstRpiCam();
  return endpoint;
}

}
#endif //OPENHD_DCAMERASHELPER_H
