#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace openhd::ground_ae {
// Sample the lower 45% of the displayed image, inside 10% horizontal margins.
// The MPP transform is applied after capture; map back to unrotated NV12 luma.
// Only 800 bytes are examined, independent of video resolution.
inline int meter(const uint8_t* luma, size_t bytes, int stride, int width,
                 int height, int orientation) {
  if (!luma || width < 40 || height < 40 || stride < width ||
      bytes < static_cast<size_t>(stride) * height) return -1;
  const int rotation = orientation >> 2;
  if (rotation != 0 && rotation != 2) return -1;
  const bool invert_y = (rotation == 2) ^ ((orientation & 2) != 0);
  unsigned histogram[256]{};
  for (int row = 0; row < 20; ++row) {
    int y = height * (2000 + 45 * (2 * row + 1)) / 4000;
    if (invert_y) y = height - 1 - y;
    for (int col = 0; col < 40; ++col) {
      const int x = width * (800 + 80 * (2 * col + 1)) / 8000;
      ++histogram[luma[static_cast<size_t>(y) * stride + x]];
    }
  }
  // Reject the darkest and brightest 10%: lamps/reflections and isolated
  // black objects should not determine the exposure of the whole ground.
  unsigned rank = 0, count = 0, sum = 0;
  for (unsigned value = 0; value < 256; ++value) {
    const unsigned end = rank + histogram[value];
    const unsigned begin_kept = std::max(rank, 80U);
    const unsigned end_kept = std::min(end, 720U);
    if (end_kept > begin_kept) {
      count += end_kept - begin_kept;
      sum += (end_kept - begin_kept) * value;
    }
    rank = end;
  }
  return count ? static_cast<int>((sum + count / 2) / count) : -1;
}

class Controller {
 public:
  void reset() { filtered = -1; }
  // V4L2 AUTO_EXPOSURE_BIAS menu 0..16 maps -2..+2 EV, quarter stops.
  // Called at most once per 800 ms; never wind up while no frame is arriving.
  int update(int luma, int current) {
    current = std::clamp(current, 0, 16);
    if (luma < 0 || luma > 255) return current;
    filtered = filtered < 0 ? luma : filtered * .5 + luma * .5;
    if (filtered < 97) return std::min(16, current + 1);
    if (filtered > 113) return std::max(0, current - 1);
    return current;
  }
 private:
  double filtered = -1;
};
}  // namespace openhd::ground_ae
