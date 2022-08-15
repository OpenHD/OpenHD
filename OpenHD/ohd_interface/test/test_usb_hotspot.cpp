//
// Created by consti10 on 20.05.22.
//

#include "USBTetherListener.h"
#include "openhd-util.hpp"

int main(int argc, char *argv[]) {

  auto cb=[](openhd::ExternalDevice external_device,bool connected){
	std::cout<<"Callback called with "<<external_device.to_string()<<" connected:"<<OHDUtil::yes_or_no(connected)<<"\n";
  };
  USBTetherListener usb_tether_listener{cb};
  usb_tether_listener.startLooping();

  OHDUtil::keep_alive_until_sigterm();

  usb_tether_listener.stopLooping();

  return 0;
}
