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
  constexpr auto zero3w = X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W;
  for (const int type : {X_CAM_TYPE_ROCK_3_HDMI_IN, X_CAM_TYPE_ROCK_3_OV5647,
                         X_CAM_TYPE_ROCK_3_IMX219, X_CAM_TYPE_ROCK_3_IMX708,
                         X_CAM_TYPE_ROCK_3_IMX462, X_CAM_TYPE_ROCK_3_IMX519,
                         X_CAM_TYPE_ROCK_3_OHD_Jaguar, X_CAM_TYPE_ROCK_3_VEYE}) {
    camera.camera_type = type;
    assert(camera.uses_zero3w_native_mpp(zero3w));
    assert(camera.get_max_video_bitrate_kbits(zero3w) == 25000);
    assert(!camera.uses_zero3w_native_mpp(X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3));
  }
  camera.camera_type = X_CAM_TYPE_EXTERNAL_IP;
  assert(!camera.uses_zero3w_native_mpp(zero3w));
  camera.camera_type = X_CAM_TYPE_LUCKFOX_AURA_IMX662;
  assert(camera.get_max_video_bitrate_kbits(X_PLATFORM_TYPE_RPI_4) == 25000);
}
