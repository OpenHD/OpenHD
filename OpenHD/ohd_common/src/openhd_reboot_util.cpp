//
// Created by consti10 on 14.02.23.
//

#include "openhd_reboot_util.h"
#include "openhd_spdlog.h"

#include <thread>

#include "openhd_util.h"

void openhd::reboot::systemctl_shutdown() {
  OHDUtil::run_command("systemctl",{"start", "poweroff.target"}, true);
}

void openhd::reboot::systemctl_reboot() {
  OHDUtil::run_command("systemctl",{"start", "reboot.target"}, true);
}

void openhd::reboot::systemctl_power(bool shutdownOnly) {
  if(shutdownOnly){
    systemctl_shutdown();
  }else{
    systemctl_reboot();
  }
}

void openhd::reboot::handle_power_command_async(std::chrono::milliseconds delay,bool shutdownOnly) {
  // This is okay, since we will restart anyways
  static auto handle=std::thread([delay,shutdownOnly]{
    std::this_thread::sleep_for(delay);
    systemctl_power(shutdownOnly);
  });
}
