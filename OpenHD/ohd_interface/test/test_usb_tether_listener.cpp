//
// Created by consti10 on 20.05.22.
//

#include "openhd-util.hpp"
#include "usb_tether_listener.h"

int main(int argc, char *argv[]) {

  OHDUtil::terminate_if_not_root();

  auto ext_devices_manager=std::make_shared<openhd::ExternalDeviceManager>();
  auto usb_tether_listener=std::make_unique<USBTetherListener>(ext_devices_manager);

  OHDUtil::keep_alive_until_sigterm();

  usb_tether_listener= nullptr;

  return 0;
}
