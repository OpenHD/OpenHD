#ifndef OPENHD_PLATFORM_H
#define OPENHD_PLATFORM_H

#include <string>

// Some things conditionally depend on the platform we are running on

// When this one shows up a bit more work has to be done to run openhd on the
// platform (probably) ;)
static constexpr int X_PLATFORM_TYPE_UNKNOWN = 0;
// Generic X86
static constexpr int X_PLATFORM_TYPE_X86 = 1;
// Numbers 10..20 are reserved for rpi
// Right now we are only interested if it is an RPI of the
// generation RPI 4 / RPI CM4 or the generation before -
// NOTE: RPI 5 is currently not supported due to the complete lack of suitable
// HW acceleration
static constexpr int X_PLATFORM_TYPE_RPI_OLD = 10;
static constexpr int X_PLATFORM_TYPE_RPI_4 = 11;
static constexpr int X_PLATFORM_TYPE_RPI_CM4 = 12;
static constexpr int X_PLATFORM_TYPE_RPI_5 = 12;

// Numbers 20..30 are reserved for rockchip
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W =
    20;  // Zero 3 W
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A = 21;
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B = 22;
static constexpr int X_PLATFORM_TYPE_ROCKCHIP_RV1126_UNDEFINED = 23;  // FUTURE

// Numbers 30..35 are reserved for allwinner
static constexpr int X_PLATFORM_TYPE_ALWINNER_X20 = 30;

// @Buldo is working on openipc / sigmastar, 36..39
static constexpr int X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED = 36;

// Numbers 40..50 are reserved for NVIDIA
static constexpr int X_PLATFORM_TYPE_NVIDIA_XAVIER = 40;

std::string x_platform_type_to_string(int platform_type);

// Depends on single threaded CPU performance & weather NEON is available
// Rough estimate
int get_fec_max_block_size_for_platform();

// All these members must not change during run time once they have been
// discovered !
struct OHDPlatform {
 public:
  explicit OHDPlatform(const int platform_type)
      : platform_type(platform_type) {}
  const int platform_type;
  [[nodiscard]] std::string to_string() const;
  static const OHDPlatform& instance();
  bool is_rpi() const;
  bool is_rock() const;
  bool is_zero3w() const;
  bool is_rock5_a() const;
  bool is_rock5_b() const;
  bool is_rock5_a_b() const;
  bool is_rpi_or_x86() const;
  // alwinner
  bool is_x20() const;
};

// We need to differentiate between rpi 4 and other pi's to use the right fec
// params.
bool platform_rpi_is_high_performance(const OHDPlatform& platform);

#endif
