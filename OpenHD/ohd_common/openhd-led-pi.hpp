//
// Created by consti10 on 12.07.22.
//

#ifndef OPENHD_OPENHD_LED_ERROR_CODES_H
#define OPENHD_OPENHD_LED_ERROR_CODES_H

#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-platform.hpp"
#include <chrono>
#include <thread>
#include <utility>

// NOTE: Some PI's allow toggling both the red and green led
// All pi's allow toggling the red led
namespace openhd::rpi{
    // so far, I have only tested this on the RPI 4 and CM4
    static void toggle_red_led(const bool on){
        int ret;
		if(!OHDFilesystemUtil::exists("/sys/class/leds/led1/brightness")){
		  std::cout<<"RPI LED1 brightness does not exist\n";
		  return;
		}
        if(on){
            OHDUtil::run_command("echo 1 > /sys/class/leds/led1/brightness",{});
        }else{
            OHDUtil::run_command("echo 0 > /sys/class/leds/led1/brightness",{});
        }
    }
    // I think the green led only supports on/off on the 4th generation pis
    static void toggle_green_led(const bool on){
        int ret;
	  if(!OHDFilesystemUtil::exists("/sys/class/leds/led0/brightness")){
		std::cout<<"RPI LED0 brightness does not exist\n";
		return;
	  }
        if(on){
            OHDUtil::run_command("echo 1 > /sys/class/leds/led0/brightness",{});
        }else{
            OHDUtil::run_command("echo 0 > /sys/class/leds/led0/brightness",{});
        }
    }
}

#endif //OPENHD_OPENHD_LED_ERROR_CODES_H
