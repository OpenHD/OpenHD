#include "libcamera_provider.h"

std::vector<Camera> LibcameraProvider::get_cameras() {
  const auto cameraManager = std::make_unique<libcamera::CameraManager>();
  cameraManager->start();
  auto lcCameras = cameraManager->cameras();

  // Filter out usb cameras
  auto rem = std::remove_if(lcCameras.begin(), lcCameras.end(), [](auto &cam) {
    return cam->id().find("/usb") != std::string::npos;
  });
  lcCameras.erase(rem, lcCameras.end());

  std::vector<Camera> ohdCameras{};
  for (auto cam : lcCameras) {
    Camera camera{};
    camera.name = cam->id();
    camera.type = CameraType::Libcamera;
    ohdCameras.push_back(camera);
  }

  // We need to stop camera manager because it can be only one run manager in process.
  // If not stop, gstream pipeline will fail
  cameraManager->stop();

  return ohdCameras;
}