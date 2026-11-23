#include "hdmi_edid.hpp"

#include <linux/v4l2-dv-timings.h>

#include <cstdarg>
#include <cstdlib>
#include <iostream>

using namespace openhd::hdmi_edid;

namespace {
Edid current{};
unsigned current_blocks = 0;
unsigned capacity_blocks = 2;
unsigned writes = 0;
unsigned queries = 0;
bool first_query_unlocked = false;
unsigned configured_timings = 0;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}
}  // namespace

extern "C" int __wrap_ioctl(int, unsigned long operation, ...) {
  va_list args;
  va_start(args, operation);
  void* argument = va_arg(args, void*);
  va_end(args);

  if (operation == VIDIOC_SUBDEV_G_EDID) {
    auto* request = static_cast<v4l2_edid*>(argument);
    request->blocks = std::min(request->blocks, current_blocks);
    std::copy_n(current.begin(), 128 * request->blocks, request->edid);
    return 0;
  }
  if (operation == VIDIOC_SUBDEV_S_EDID) {
    auto* request = static_cast<v4l2_edid*>(argument);
    if (request->blocks > capacity_blocks) {
      request->blocks = capacity_blocks;
      errno = E2BIG;
      return -1;
    }
    current_blocks = request->blocks;
    std::fill(current.begin(), current.end(), 0);
    std::copy_n(request->edid, 128 * request->blocks, current.begin());
    ++writes;
    return 0;
  }
  if (operation == VIDIOC_SUBDEV_QUERY_DV_TIMINGS) {
    ++queries;
    if (first_query_unlocked && queries == 1) {
      errno = ENOLINK;
      return -1;
    }
    const v4l2_dv_timings detected = V4L2_DV_BT_CEA_1280X720P60;
    *static_cast<v4l2_dv_timings*>(argument) = detected;
    return 0;
  }
  if (operation == VIDIOC_SUBDEV_S_DV_TIMINGS) {
    ++configured_timings;
    return 0;
  }
  errno = ENOTTY;
  return -1;
}

int main() {
  for (const auto& mode : modes) {
    const auto edid = edid_for(mode);
    require(edid[126] == 1 && edid[128] == 2, "Missing CTA extension");
    require(edid[133] == (0x80 | mode.vic), "Selected mode is not native");
    for (int block = 0; block < 2; ++block) {
      const auto start = edid.begin() + block * 128;
      require(std::accumulate(start, start + 128, 0u) % 256 == 0,
              "Invalid EDID checksum");
    }
  }

  const auto& mode = *find_mode(1280, 720, 60);
  require(apply(0, mode) == ApplyResult::Written && writes == 1,
          "Initial HDMI mode was not programmed");
  require(apply(0, mode) == ApplyResult::Unchanged && writes == 1,
          "An identical mode should not pulse hotplug again");

  capacity_blocks = 1;
  const auto& lower_rate = *find_mode(1280, 720, 30);
  require(apply(0, lower_rate) == ApplyResult::Written && current_blocks == 1 &&
              current[126] == 0 && writes == 2,
          "A one-block receiver must receive the base EDID");
  require(apply(0, lower_rate) == ApplyResult::Unchanged && writes == 2,
          "A one-block receiver should not be rewritten on restart");

  require(!wait_for_input(0, lower_rate, 74500000, false) &&
              configured_timings == 0,
          "Do not start capture in a different HDMI mode");
  queries = 0;
  first_query_unlocked = true;
  require(wait_for_input(0, mode, 74500000, true) && queries == 2 &&
              configured_timings == 1,
          "Retry HDMI lock after updating EDID and configure matching timings");
}
