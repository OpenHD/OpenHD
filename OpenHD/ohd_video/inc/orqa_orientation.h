#pragma once

// Hantro exposes clockwise rotation and a horizontal mirror. Compose a
// vertical flip as a horizontal mirror plus a half turn, keeping input-sized
// capture buffers and swapping encoded dimensions only for quarter turns.
struct OrqaOrientation {
  int rotation;
  bool mirror;
};

constexpr OrqaOrientation orqa_orientation(int rotation, int flip) {
  const bool horizontal = (flip & 1) != 0;
  const bool vertical = (flip & 2) != 0;
  return {(rotation + (vertical ? 180 : 0)) % 360,
          horizontal != vertical};
}
