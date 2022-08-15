//
// Created by consti10 on 15.08.22.
//

#include "openhd-led-error-codes.h"
#include "openhd-util.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[]) {

  while(true){
	openhd::rpi::toggle_red_led(false);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	openhd::rpi::toggle_red_led(false);
	std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
