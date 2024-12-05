/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_LIBCAMERA_PROVIDER_H
#define OPENHD_LIBCAMERA_PROVIDER_H

// NOTE: This header just hides away libcamera provider if library is not found
// at compile time.
#ifdef OPENHD_LIBCAMERA_PRESENT
#include <libcamera/libcamera.h>

#include "camera.hpp"
#include "openhd_spdlog.h"

// Namespace for rpi csi camera(s) detection using libcamera
namespace openhd::libcameradetect {

// TODO check more sensors here or use a regex / better method
static std::string get_sensor_name_from_cam_id(const std::string& cam_id) {
  std::string ret = "unknown";
  if (OHDUtil::contains(cam_id, "imx477")) {
    ret = "imx477";  // (rpi) HQ cam or clone
  } else if (OHDUtil::contains(cam_id, "imx219")) {
    ret = "imx219";  // rpi cam v2 (or clone)
  } else if (OHDUtil::contains(cam_id, "ov5647")) {
    ret = "ov5647";  // rpi cam v1 (or clone)
  } else if (OHDUtil::contains(cam_id, "imx415")) {
    ret = "imx415";
  } else if (OHDUtil::contains(cam_id, "imx462")) {
    // arducam low light beast
    ret = "imx462";
  } else if (OHDUtil::contains(cam_id, "imx298")) {
    // arducam
    ret = "imx298";
  } else if (OHDUtil::contains(cam_id, "imx230")) {
    // arducam
    ret = "imx230";
  } else if (OHDUtil::contains_after_uppercase(cam_id, "AR0234")) {
    ret = "AR0234";
  } else if (OHDUtil::contains_after_uppercase(cam_id, "AR1820HS")) {
    // arducam
    // https://www.uctronics.com/arducam-18mp-ar1820hs-camera-module-for-raspberry-pi-pivariety.html
    ret = "AR1820HS";
  } else if (OHDUtil::contains_after_uppercase(cam_id, "OV2311")) {
    // arducam
    // https://www.uctronics.com/2mp-global-shutter-ov2311-mono-camera-modules-pivariety.html
    ret = "OV2311";
  }
  return ret;
}

static std::vector<Camera> get_csi_cameras() {
  const auto cameraManager = std::make_unique<libcamera::CameraManager>();
  openhd::log::get_default()->info("Libcamera reports version:" +
                                   cameraManager->version());
  cameraManager->start();
  auto lcCameras = cameraManager->cameras();

  std::vector<Camera> ohdCameras{};
  for (const auto& cam : lcCameras) {
    const auto cam_id = cam->id();
    openhd::log::get_default()->info("Libcamera reports cam with cam_id [{}]",
                                     cam_id);
    // We do not want usb cameras from libcamera
    if (cam_id.find("/usb") == std::string::npos) {
      openhd::log::get_default()->info("Libcamera CSI cam found, cam_id:[{}]",
                                       cam_id);
      Camera camera{};
      camera.name = cam_id;
      camera.type = CameraType::RPI_CSI_LIBCAMERA;
      camera.sensor_name = get_sensor_name_from_cam_id(cam_id);
      ohdCameras.push_back(camera);
    }
  }
  // This should free all the shared pointers we might still have, such that
  // hopefully we can call stop() later without errors. The documentation is a
  // bit dubios here -
  // https://libcamera.org/api-html/classlibcamera_1_1CameraManager.html
  // Before stopping the camera manager the caller is responsible for making
  // sure all cameras provided by the manager are returned to the manager. well,
  // I don't think we call get so to say but we kept the shared pointer(s)
  // around before calling stop() previously Here we get rid of all the shared
  // pointers we still hold From CameraManager.c (not the public doc): Once the
  // application has released all the
  // * references it held to cameras, the camera manager can be stopped with
  // * stop().
  lcCameras.resize(0);

  // We need to stop camera manager because it can be only one run manager in
  // process. If not stop, gstream pipeline will fail
  cameraManager->stop();

  return ohdCameras;
}

}  // namespace openhd::libcameradetect

#endif  // OPENHD_LIBCAMERA_PRESENT

#endif  // OPENHD_LIBCAMERA_PROVIDER_H
