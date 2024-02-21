//
// Created by consti10 on 20.05.22.
//

#include "openhd_util.h"
#include "usb_tether_listener.h"

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  auto usb_tether_listener = std::make_unique<USBTetherListener>();

  OHDUtil::keep_alive_until_sigterm();

  usb_tether_listener = nullptr;

  return 0;
}
