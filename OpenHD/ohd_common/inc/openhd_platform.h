#ifndef OPENHD_PLATFORM_H
#define OPENHD_PLATFORM_H

#include <string>

#include "openhd_spdlog.h"

/**
 * Util to discover and then store the platform we are running on.
 */

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
std::string platform_type_to_string(PlatformType platform_type);

enum class BoardType {
  Unknown,
  GenericPC,
  RaspberryPiZero,
  RaspberryPiZeroW,
  RaspberryPiZero2W,
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
  Allwinner,
  PynqZ1,
  PynqZ2,
  X3DRSolo,
  RK3588,
  RV1109,
  RV1126
};
std::string board_type_to_string(BoardType board_type);

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
  static const OHDPlatform& instance();
};

// We need to differentiate between rpi 4 and other pi's to use the right fec params.
static bool platform_rpi_is_high_performance(const OHDPlatform& platform){
  assert(platform.platform_type==PlatformType::RaspberryPi);
  const auto rpi_board_type=platform.board_type;
  if(rpi_board_type==BoardType::RaspberryPi4B || rpi_board_type==BoardType::RaspberryPiCM4){
    return true;
  }
  return false;
}

void write_platform_manifest(const OHDPlatform &ohdPlatform);

namespace DPlatform{

std::shared_ptr<OHDPlatform> discover();

}


#endif
