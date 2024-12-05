/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

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
