//
// Created by consti10 on 20.05.22.
//
#include <csignal>
#include "USBTetherListener.h"

int main(int argc, char *argv[]) {

  USBTetherListener usb_tether_listener{nullptr};
  usb_tether_listener.startLooping();

  OHDUtil::keep_alive_until_sigterm();

  usb_tether_listener.stopLooping();

  return 0;
}
