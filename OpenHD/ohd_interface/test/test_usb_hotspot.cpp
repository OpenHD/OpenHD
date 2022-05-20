//
// Created by consti10 on 20.05.22.
//
#include "USBTether.hpp"

int main(int argc, char *argv[]) {

  USBTetherListener usb_tether_listener{nullptr};
  usb_tether_listener.loopInfinite();

}
