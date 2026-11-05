#pragma once

static constexpr int ROCKCHIP_IMPL_GSTREAMER = 0;
static constexpr int ROCKCHIP_IMPL_NATIVE_MPP = 1;

namespace openhd::video {
enum class RockchipBackend { Unavailable, NativeMpp, EmergencyGStreamer };

// Never silently replace the independent recording encoder with a TX-stream tee.
constexpr RockchipBackend resolve_rockchip_backend(int selected, bool native_mpp,
                                                   bool gstreamer) {
  if (selected == ROCKCHIP_IMPL_NATIVE_MPP)
    return native_mpp ? RockchipBackend::NativeMpp : RockchipBackend::Unavailable;
  if (selected == ROCKCHIP_IMPL_GSTREAMER)
    return gstreamer ? RockchipBackend::EmergencyGStreamer : RockchipBackend::Unavailable;
  return RockchipBackend::Unavailable;
}
}  // namespace openhd::video
