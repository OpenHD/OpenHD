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

#include <rc/JoystickReader.h>

#include <cassert>
#include <csignal>
#include <iostream>
#include <memory>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

int main() {
  std::shared_ptr<spdlog::logger> m_console = openhd::log::get_default();
  assert(m_console);

  m_console->debug("test_joystick_reader");

  auto joystick_reader = std::make_unique<JoystickReader>();

  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << JoystickReader::curr_state_to_string(
                     joystick_reader->get_current_state())
              << "\n";
  }

  return 0;
}