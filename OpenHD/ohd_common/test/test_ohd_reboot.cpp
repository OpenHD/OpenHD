//
// Created by consti10 on 26.04.22.
// Test the c++-implementation for the reboot / shutdown commands
// Probably needs to be run with sudo
//

#include <thread>

#include "openhd_reboot_util.h"

int main() {
  // RebootUtil::handlePowerCommand(false);
  openhd::reboot::handle_power_command_async(std::chrono::seconds(3), false);
  std::this_thread::sleep_for(std::chrono::seconds(10));
}