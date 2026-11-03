#include "camera.hpp"
#include <cassert>

int main() {
  XCamera camera{};
  camera.index = 0;
  camera.camera_type = X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708;
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_4) == 25000);
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_CM4) == 25000);
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_5) == 30000);
  camera.index = 1;
  camera.camera_type = X_CAM_TYPE_USB_INFIRAY_T2;
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_4) == 50000);
  camera.camera_type = X_CAM_TYPE_LUCKFOX_AURA_IMX662;
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_4) == 25000);
}
