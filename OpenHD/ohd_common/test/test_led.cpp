//
// Created by consti10 on 15.08.22.
//

#include <chrono>
#include <thread>
#include <iostream>

#include "openhd_led.h"
#include "openhd_util.h"

int main(int argc, char *argv[]) {

  openhd::LEDManager::instance().set_red_led_status(openhd::LEDManager::STATUS_OFF);
  openhd::LEDManager::instance().set_green_led_status(openhd::LEDManager::STATUS_OFF);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  openhd::LEDManager::instance().set_red_led_status(openhd::LEDManager::STATUS_ON);
  openhd::LEDManager::instance().set_green_led_status(openhd::LEDManager::STATUS_ON);

  std::this_thread::sleep_for(std::chrono::seconds(10));
}
