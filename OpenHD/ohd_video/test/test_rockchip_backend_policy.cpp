#include "rockchip_backend_policy.h"

using openhd::video::RockchipBackend;
using openhd::video::resolve_rockchip_backend;
static_assert(resolve_rockchip_backend(ROCKCHIP_IMPL_NATIVE_MPP, true, true) ==
              RockchipBackend::NativeMpp);
static_assert(resolve_rockchip_backend(ROCKCHIP_IMPL_NATIVE_MPP, false, true) ==
              RockchipBackend::Unavailable); // Never silently use a TX-stream tee.
static_assert(resolve_rockchip_backend(ROCKCHIP_IMPL_GSTREAMER, true, true) ==
              RockchipBackend::EmergencyGStreamer);
static_assert(resolve_rockchip_backend(ROCKCHIP_IMPL_GSTREAMER, true, false) ==
              RockchipBackend::Unavailable);
static_assert(resolve_rockchip_backend(ROCKCHIP_IMPL_NATIVE_MPP, true, false) ==
              RockchipBackend::NativeMpp);
static_assert(resolve_rockchip_backend(42, true, true) == RockchipBackend::Unavailable);
int main() {}
