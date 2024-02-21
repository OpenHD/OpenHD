//
// Created by consti10 on 07.11.22.
//
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