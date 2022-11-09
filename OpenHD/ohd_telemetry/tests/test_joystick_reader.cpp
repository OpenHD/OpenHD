//
// Created by consti10 on 07.11.22.
//
#include <rc/JoystickReader.h>

#include <csignal>
#include <iostream>
#include <memory>
#include "openhd-spdlog.hpp"

int main() {
  std::shared_ptr<spdlog::logger> m_console=spdlog::stdout_color_mt("main");
  assert(m_console);
  m_console->set_level(spd::level::debug);

  m_console->debug("test_joystick_reader");

  auto joystick_reader=std::make_unique<JoystickReader>(JoystickReader::get_default_channel_mapping());

  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout<<JoystickReader::curr_state_to_string(joystick_reader->get_current_state())<<"\n";
  }

  return 0;
}