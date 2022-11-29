//
// Created by consti10 on 29.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_GPIO_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_GPIO_H_

#include "openhd-util.hpp"

namespace openhd::rpi{

// RPI GPIO26: Used as a reset button

// After configure: Connect gpio26 to ground -> reports 0
// Do not connect gpio26 to ground -> reports 1
static void gpio26_configure(){
  OHDUtil::run_command("raspi-gpio",{"set","26","ip","pu"});
};

static bool gpio26_user_wants_reset_frequencies(){
    const auto res_opt=OHDUtil::run_command_out("raspi-gpio get 26");
    if(res_opt){
      const auto res=res_opt.value();
      if(OHDUtil::contains(res,"GPIO 26: level=0 fsel=0 func=INPUT pull=UP")){
        openhd::log::get_default()->info("GPIO26 pull UP and level=0, user_wants_reset_frequencies");
        return true;
      }
    }
    return false;
}
}

}


#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_GPIO_H_
