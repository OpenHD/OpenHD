#include "libcamera_provider.h"
std::vector<Camera> LibcameraProvider::get_cameras() {
  throw std::runtime_error("Libcamera not available");
}