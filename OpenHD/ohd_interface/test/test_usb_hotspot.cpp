//
// Created by consti10 on 20.05.22.
//
#include <csignal>
#include "USBTetherListener.h"

int main(int argc, char *argv[]) {

  USBTetherListener usb_tether_listener{nullptr};
  usb_tether_listener.startLooping();

  static bool quit=false;
  signal(SIGTERM, [](int sig){ quit= true;});
  while (!quit){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"X\n";
  }
  usb_tether_listener.stopLooping();

  return 0;
}
