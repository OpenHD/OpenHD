//
// Created by buldo on 15.08.22.
//

#ifndef OPENHD_LIBCAMERA_PROVIDER_H
#define OPENHD_LIBCAMERA_PROVIDER_H

// NOTE: This header just hides away libcamera provider if library is not found at compile time.
#ifdef OPENHD_LIBCAMERA_PRESENT
#include <libcamera/libcamera.h>
#include "openhd-camera.hpp"

class LibcameraProvider {
 public:
  static std::vector<Camera> get_cameras(){
	const auto cameraManager = std::make_unique<libcamera::CameraManager>();
	cameraManager->start();
	auto lcCameras = cameraManager->cameras();

	std::vector<Camera> ohdCameras{};
	for (const auto& cam : lcCameras) {
          // We do not want usb cameras from libcamera
          if(cam->id().find("/usb") == std::string::npos){
            Camera camera{};
            camera.name = cam->id();
            camera.type = CameraType::Libcamera;
            ohdCameras.push_back(camera);
          }
	}
        // This should free all the shared pointers we might still have, such that hopefully
        // we can call stop() later without erros. The documentation is a bit dubios here -
        // https://libcamera.org/api-html/classlibcamera_1_1CameraManager.html
        // Before stopping the camera manager the caller is responsible for making sure all cameras provided by the manager are returned to the manager.
        // well, I don't think we call get so to say but we kept the shared pointer(s) around before calling stop() previously
        lcCameras.resize(0);

	// We need to stop camera manager because it can be only one run manager in process.
	// If not stop, gstream pipeline will fail
	cameraManager->stop();

	return ohdCameras;
  }
};

#endif //OPENHD_LIBCAMERA_PRESENT

#endif  // OPENHD_LIBCAMERA_PROVIDER_H
