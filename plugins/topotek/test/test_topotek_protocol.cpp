#include <cassert>
#include <iostream>

#include "topotek_protocol.h"

int main() {
  assert(topotek::add_checksum("#TPUM2wZMC00") == "#TPUM2wZMC005C");
  assert(topotek::make_command('G', "PTZ", "00") == "#TPPG2wPTZ0065");
  assert(topotek::make_command('D', "REC", "0A") == "#TPPD2wREC0A4F");
  assert(topotek::make_command('D', "CAP", "01") == "#TPPD2wCAP0139");
  assert(topotek::make_gimbal_rate_command(0, 0) ==
         "#tpPG4wGSM0000F0");
  assert(topotek::make_gimbal_rate_command(-30, 30) ==
         "#tpPG4wGSM1EE21D");
  assert(topotek::make_gimbal_angle_command(-50.0F, 10.0F, 50) ==
         "#tpPGCwGAM03E832EC7832CE");
  std::cout << "Topotek protocol command packets passed\n";
  return 0;
}
