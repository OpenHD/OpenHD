#include "libcamera_provider.h"

LibcameraProvider::LibcameraProvider() {

}

std::vector<Camera> LibcameraProvider::get_cameras() {
  const auto cameraManager = std::make_unique<libcamera::CameraManager>();
  cameraManager->start();
  const auto lcCameras = cameraManager->cameras();

  std::vector<Camera> ohdCameras{};
  for (auto cam : lcCameras) {
    Camera camera{};
    camera.name = cam->id();
    camera.type = CameraType::Libcamera;
    ohdCameras.push_back(camera);
  }
  cameraManager->stop();

  return ohdCameras;
}