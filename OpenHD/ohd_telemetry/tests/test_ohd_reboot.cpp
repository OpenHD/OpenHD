//
// Created by consti10 on 26.04.22.
// Test the c++-implementation for the reboot / shutdown commands
// Probably needs to be run with sudo
//

#include "../src/internal//RebootUtil.hpp"
#include <thread>

int main() {
  //RebootUtil::handlePowerCommand(false);
  RebootUtil::handle_power_command_async(std::chrono::seconds(3), false);
  std::this_thread::sleep_for(std::chrono::seconds(10));
}