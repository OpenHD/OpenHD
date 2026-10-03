#pragma once

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdint>
#include <mutex>
#include <optional>

// GX ISP protocol and registers documented by VEYE:
// https://wiki.veye.cc/index.php?title=Gx_Series_MIPI_Camera_Register_Map
namespace veye_gx {
struct Control {
  const char* id;
  uint16_t reg;
  int min;
  int max;
};
inline constexpr Control controls[] = {
    {"GX_DAYNIGHT", 0x0474, 0, 2},
    {"GX_IRCUT_DIR", 0x0478, 0, 1},
    {"GX_IRCUT_TIMER", 0x0480, 0, 1},
    {"GX_EXPOSURE", 0x0c04, 0, 2},
    {"GX_AE_TARGET", 0x0c08, 0, 255},
    {"GX_AE_STRATEGY", 0x0c0c, 0, 1},
    {"GX_SHUTTER_US", 0x0c10, 16, 33333},
    {"GX_AE_MAX_US", 0x0c14, 16, 33333},
    // Gain registers use tenths of a dB, including digital gain above 45.3 dB.
    {"GX_GAIN", 0x0c20, 0, 720},
    {"GX_AE_MAX_GAIN", 0x0c24, 0, 720},
    {"GX_WB_MODE", 0x0c3c, 0, 2},
    {"GX_WB_RED", 0x0c48, 0, 4095},
    {"GX_WB_BLUE", 0x0c4c, 0, 4095},
    {"GX_SHARPNESS", 0x0ca4, 0, 255},
    {"GX_DENOISE_2D", 0x0ca8, 0, 255},
    {"GX_DENOISE_3D", 0x0cac, 0, 255},
    {"GX_SATURATION", 0x0cb0, 0, 100},
    {"GX_CONTRAST", 0x0cb4, 0, 100},
    {"GX_HUE", 0x0cb8, 0, 100},
    {"GX_GAMMA", 0x0cd0, 0, 11},
    {"GX_DRC", 0x0cd4, 0, 255},
};

class Device {
 public:
  Device() : fd(open("/dev/i2c-10", O_RDWR | O_CLOEXEC)) {}
  ~Device() { if (fd >= 0) close(fd); }
  bool transfer(i2c_msg* msgs, unsigned count) {
    i2c_rdwr_ioctl_data transaction{msgs, count};
    return fd >= 0 && ioctl(fd, I2C_RDWR, &transaction) == int(count);
  }
  static uint8_t checksum(const uint8_t* data, int length) {
    uint8_t result = 0;
    for (int i = 0; i < length; ++i) result ^= data[i];
    return result;
  }
  std::optional<int> read(uint16_t reg) {
    uint8_t prepare[]{0xbc, uint8_t(reg >> 8), uint8_t(reg), 0};
    prepare[3] = checksum(prepare, 3);
    i2c_msg pre{0x3b, 0, sizeof(prepare), prepare};
    if (!transfer(&pre, 1)) return {};
    usleep(20000);
    uint8_t request[]{0xac, uint8_t(reg >> 8), uint8_t(reg)};
    uint8_t response[6]{};
    i2c_msg msgs[]{{0x3b, 0, sizeof(request), request},
                   {0x3b, I2C_M_RD, sizeof(response), response}};
    if (!transfer(msgs, 2) || checksum(response, 5) != response[5]) return {};
    return int((uint32_t(response[0]) << 24) |
               (uint32_t(response[1]) << 16) |
               (uint32_t(response[2]) << 8) | response[3]);
  }
  bool write(uint16_t reg, int value) {
    uint8_t bytes[]{0xab, uint8_t(reg >> 8), uint8_t(reg),
                    uint8_t(value >> 24), uint8_t(value >> 16),
                    uint8_t(value >> 8), uint8_t(value), 0};
    bytes[7] = checksum(bytes, 7);
    i2c_msg msg{0x3b, 0, sizeof(bytes), bytes};
    if (!transfer(&msg, 1)) return false;
    usleep(20000);
    const auto actual = read(reg);
    return actual && *actual == value;
  }
 private:
  int fd;
};
inline std::mutex mutex;
inline std::optional<int> read(const Control& control) {
  std::lock_guard<std::mutex> lock(mutex);
  return Device().read(control.reg);
}
inline bool set(const Control& control, int value) {
  if (value < control.min || value > control.max) return false;
  if ((control.reg == 0x0c04 || control.reg == 0x0c3c) && value == 1)
    return false;
  std::lock_guard<std::mutex> lock(mutex);
  return Device().write(control.reg, value);
}
}  // namespace veye_gx
