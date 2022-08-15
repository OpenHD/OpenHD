//
// Created by consti10 on 15.08.22.
//

#include "openhd-led-error-codes.h"
#include "openhd-util.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[]) {

  const auto start1=std::chrono::steady_clock::now();
  while((std::chrono::steady_clock::now()-start1)<std::chrono::seconds(10)){
	openhd::rpi::toggle_red_led(false);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	openhd::rpi::toggle_red_led(true);
	std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  const auto start2=std::chrono::steady_clock::now();
  while((std::chrono::steady_clock::now()-start2)<std::chrono::seconds(10)){
	openhd::rpi::toggle_green_led(false);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	openhd::rpi::toggle_green_led(true);
	std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
