//
// Created by consti10 on 19.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_

#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"

// USB hotspot (USB Tethering)
namespace USBHotspot{

static void enable(){
  while (true){
	const char* connectedDevice="/sys/class/net/usb0";
	if(OHDFilesystemUtil::exists(connectedDevice)){
	  std::cout<<"Found USB tethering device\n";

	  // configure its ip

	}
  }
}

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
