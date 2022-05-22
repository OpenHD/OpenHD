//
// Created by consti10 on 21.05.22.
//

#include "USBTetherListener.h"

void USBTetherListener::setDeviceIpLocked(std::string newDeviceIp) {
  std::lock_guard<std::mutex> guard(device_ip_mutex);
  device_ip=std::move(newDeviceIp);
}

std::vector<std::string> USBTetherListener::getConnectedTetherIPsLocked() {
  std::lock_guard<std::mutex> guard(device_ip_mutex);
  if(device_ip.empty()){
	return {};
  }
  return std::vector<std::string>{device_ip};
}

void USBTetherListener::loopInfinite() {
  while (true){
	connectOnce();
  }
}

void USBTetherListener::connectOnce() {
  const char* connectedDevice="/sys/class/net/usb0";
  // in regular intervals, check if the devices becomes available - if yes, the user connected a ethernet hotspot device.
  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"Checking for USB tethering device\n";
	if(OHDFilesystemUtil::exists(connectedDevice)) {
	  std::cout << "Found USB tethering device\n";
	  break;
	}
  }
  // Configure the detected USB tether device (not sure if needed)
  OHDUtil::run_command("dhclient",{"usb0"});

  // now we find the IP of the connected device so we can forward video usw to it
  const auto ip_opt=OHDUtil::run_command_out("ip route show 0.0.0.0/0 dev usb0 | cut -d\\  -f3");
  if(ip_opt!=std::nullopt){
	const auto ip=ip_opt.value();
	setDeviceIpLocked(ip);
	std::cout<<"Found ip:["<<device_ip<<"]\n";
	if(ip_callback){
	  ip_callback(false,device_ip);
	}
  }else{
	std::cerr<<"USBHotspot find ip no success\n";
	return;
  }
  // check in regular intervals if the tethering device disconnects.
  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"Checking if USB tethering device disconnected\n";
	if(!OHDFilesystemUtil::exists(connectedDevice)){
	  std::cout<<"USB Tether device disconnected\n";
	  break;
	}
  }
  if(ip_callback){
	ip_callback(true,device_ip);
  }
  device_ip="";
}

