//
// Created by consti10 on 15.08.22.
//

#include "openhd-led-codes.hpp"
#include "openhd-util.hpp"
#include "openhd-platform-discover.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[]) {

  auto platform=DPlatform::discover();
  std::cout<<"First ground\n";
  auto blinker=std::make_unique<openhd::GreenLedAliveBlinker>(*platform,false);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  std::cout<<"Now air\n";
  blinker= nullptr;
  blinker=std::make_unique<openhd::GreenLedAliveBlinker>(*platform,true);
  std::this_thread::sleep_for(std::chrono::seconds(10));

  /*const auto start1=std::chrono::steady_clock::now();
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
  }*/
}
