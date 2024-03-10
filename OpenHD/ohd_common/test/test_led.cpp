//
// Created by consti10 on 15.08.22.
//

#include <chrono>
#include <iostream>
#include <thread>

#include "openhd_led.h"
#include "openhd_util.h"

int main(int argc, char *argv[]) {
  while (true) {
    std::cout << "Set LEDs off" << std::endl;
    openhd::LEDManager::instance().set_red_led_status(
        openhd::LEDManager::STATUS_OFF);
    openhd::LEDManager::instance().set_green_led_status(
        openhd::LEDManager::STATUS_OFF);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Set LEDs ON" << std::endl;
    openhd::LEDManager::instance().set_red_led_status(
        openhd::LEDManager::STATUS_ON);
    openhd::LEDManager::instance().set_green_led_status(
        openhd::LEDManager::STATUS_ON);

    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}
