#ifndef OPENHD_PLATFORM_H
#define OPENHD_PLATFORM_H

#include <string>
#include <sstream>
#include <fstream>

#include "openhd-util.hpp"

enum class PlatformType {
  Unknown,
  PC,
  RaspberryPi,
  Jetson,
  Allwinner,
  iMX6,
  Rockchip,
  Zynq,
};
inline std::string platform_type_to_string(PlatformType platform_type) {
  switch (platform_type) {
    case PlatformType::Jetson: return "jetson";
    case PlatformType::RaspberryPi: return "raspberrypi";
    case PlatformType::Allwinner: return "allwinner";
    case PlatformType::iMX6: return "imx6";
    case PlatformType::Zynq: return "zynq";
    case PlatformType::PC: return "pc";
    case PlatformType::Rockchip: return "rockchip";
    default: return "unknown";
  }
}

enum class BoardType {
  Unknown,
  GenericPC,
  RaspberryPiZero,
  RaspberryPiZeroW,
  RaspberryPi2B,
  RaspberryPi3A,
  RaspberryPi3APlus,
  RaspberryPi3B,
  RaspberryPi3BPlus,
  RaspberryPiCM,
  RaspberryPiCM3,
  RaspberryPiCM3Plus,
  RaspberryPiCM4,
  RaspberryPi4B,
  JetsonNano,
  JetsonTX1,
  JetsonTX2,
  JetsonNX,
  JetsonAGX,
  NanoPiDuo2,
  PynqZ1,
  PynqZ2,
  X3DRSolo,
  RK3588,
  RV1109,
  RV1126
};
inline std::string board_type_to_string(BoardType board_type) {
  switch (board_type) {
    case BoardType::GenericPC: return "generic-pc";
    case BoardType::RaspberryPiZero: return "pizero";
    case BoardType::RaspberryPiZeroW: return "pizerow";
    case BoardType::RaspberryPi2B: return "pi2b";
    case BoardType::RaspberryPi3A: return "pi3a";
    case BoardType::RaspberryPi3APlus: return "pi3aplus";
    case BoardType::RaspberryPi3B: return "pi3b";
    case BoardType::RaspberryPi3BPlus: return "pi3bplus";
    case BoardType::RaspberryPiCM: return "picm";
    case BoardType::RaspberryPiCM3: return "picm3";
    case BoardType::RaspberryPiCM3Plus: return "picm3plus";
    case BoardType::RaspberryPiCM4: return "picm4";
    case BoardType::RaspberryPi4B: return "pi4b";
    case BoardType::JetsonNano: return "jetson-nano";
    case BoardType::JetsonTX1: return "jetson-tx1";
    case BoardType::JetsonTX2: return "jetson-tx2";
    case BoardType::JetsonNX: return "jetson-nx";
    case BoardType::JetsonAGX: return "jetson-agx";
    case BoardType::NanoPiDuo2: return "nanopi-duo2";
    case BoardType::PynqZ1: return "pynqz1";
    case BoardType::PynqZ2: return "pynqz2";
    case BoardType::X3DRSolo: return "3dr-solo";
    case BoardType::RK3588: return "rk3588";
    case BoardType::RV1109: return "rv1109";
    case BoardType::RV1126: return "rv1126";
    default: return "unknown";
  }
}

// All these members must not change during run time once they have been discovered !
struct OHDPlatform {
 public:
  explicit OHDPlatform(PlatformType platform_type = PlatformType::Unknown,BoardType board_type = BoardType::Unknown):
	  platform_type(platform_type),board_type(board_type){}
  // The platform we are running on, for example rpi, jetson
  const PlatformType platform_type;
  // The board type we are running on, for example rpi 3B+
  const BoardType board_type;
  [[nodiscard]] std::string to_string()const{
    return fmt::format("[{}:{}]",platform_type_to_string(platform_type),board_type_to_string(board_type));
  }
};

static bool platform_rpi_is_high_performance(const OHDPlatform& platform){
  assert(platform.platform_type==PlatformType::RaspberryPi);
  const auto rpi_board_type=platform.board_type;
  if(rpi_board_type==BoardType::RaspberryPi4B || rpi_board_type==BoardType::RaspberryPiCM4){
    return true;
  }
  return false;
}


#endif
