//
// Created by consti10 on 19.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_

#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"
#include <thread>
#include <chrono>

// USB hotspot (USB Tethering)
// This was created by translating the tether_functions.sh script form wifibroadcast-scripts into c++.
namespace USBHotspot{

static void enable(){
  // in regular intervals, check if the devices becomes available - if yes, the user connected a ethernet hotspot device.
  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	const char* connectedDevice="/sys/class/net/usb0";
	// When the file above becomes available, we almost 100% have a usb device with "mobile hotspot"
	// connected.
	std::cout<<"Checking for USB tethering device\n";
	if(OHDFilesystemUtil::exists(connectedDevice)){
	  std::cout<<"Found USB tethering device\n";
	  // Not sure if this is needed, configure the IP ?!
	  OHDUtil::run_command("dhclient",{"usb0"});

	  // now we find the IP of the connected device so we can forward video usw to it
	  const auto ip_opt=OHDUtil::run_command_out("ip route show 0.0.0.0/0 dev usb0 | cut -d\\  -f3");

	  if(ip_opt!=std::nullopt){
		const std::string ip=ip_opt.value();
		std::cout<<"Found ip:["<<ip<<"]\n";
	  }else{
		std::cerr<<"USBHotspot find ip no success\n";
		return;
	  }
	  // Now that we have hotspot setup and queried the IP,
	  // check in regular intervals if the tethering device is disconnected.
	  while (true){
		std::cout<<"Checking if USB tethering device disconnected\n";
		if(!OHDFilesystemUtil::exists(connectedDevice)){
		  std::cout<<"USB Tether device disconnected\n";
		  return;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	  }
	}
  }
}

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
