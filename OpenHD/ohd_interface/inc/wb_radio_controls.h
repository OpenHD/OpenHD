#pragma once

// Shared by the normal link and the RF laboratory GUI. Keep these lightweight
// so the portable Windows build uses exactly the same limits and power curve.
namespace openhd {
inline constexpr int WB_VIDEO_DEFAULT_FEC_PERCENT = 20;
inline constexpr int WB_TX_POWER_LEVEL_MIN = 0;
inline constexpr int WB_TX_POWER_LEVEL_MAX_NORMAL = 100;
inline constexpr int WB_TX_POWER_LEVEL_MAX_OVERDRIVE = 150;
inline constexpr int WB_TX_POWER_LEVEL_STEP = 10;
inline constexpr int WB_TX_POWER_LEVEL_20 = 20;
inline constexpr int WB_TX_POWER_LEVEL_40 = 40;
inline constexpr int WB_TX_POWER_LEVEL_60 = 60;
inline constexpr int WB_TX_POWER_LEVEL_80 = 80;
inline constexpr int WB_TX_POWER_LEVEL_100 = 100;
inline constexpr bool is_valid_channel_width(unsigned width) {
  return width == 10 || width == 20 || width == 40;
}
inline constexpr bool is_valid_mcs_index(unsigned mcs) { return mcs <= 31; }
inline constexpr bool is_valid_stbc(int stbc) { return stbc >= 0 && stbc <= 3; }
inline constexpr bool is_valid_fec_block_length(int block) {
  return block >= 0 && block < 100;
}
inline constexpr bool is_valid_fec_percentage_value(int percent) {
  return percent > 0 && percent <= 400;
}
inline constexpr bool is_valid_power_level(int level) {
  return level >= WB_TX_POWER_LEVEL_MIN &&
         level <= WB_TX_POWER_LEVEL_MAX_OVERDRIVE &&
         level % WB_TX_POWER_LEVEL_STEP == 0;
}
inline constexpr int devourer_power_offset_qdb(int level) {
  if (level <= 0) return -127;
  if (level >= 100) return 0;
  int x0 = 0, y0 = -127, x1 = 20, y1 = -64;
  if (level > 20 && level <= 40) {
    x0 = 20;
    y0 = -64;
    x1 = 40;
    y1 = -40;
  } else if (level > 40 && level <= 60) {
    x0 = 40;
    y0 = -40;
    x1 = 60;
    y1 = -24;
  } else if (level > 60 && level <= 80) {
    x0 = 60;
    y0 = -24;
    x1 = 80;
    y1 = -12;
  } else if (level > 80) {
    x0 = 80;
    y0 = -12;
    x1 = 100;
    y1 = 0;
  }
  return y0 + ((y1 - y0) * (level - x0) + (x1 - x0) / 2) / (x1 - x0);
}
}  // namespace openhd
