//
// Created by consti10 on 21.05.22.
//

#include "USBTetherListener.h"
#include <arpa/inet.h>

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
  while (!loopThreadStop){
	connectOnce();
  }
}

void USBTetherListener::connectOnce() {
  const char* connectedDevice="/sys/class/net/usb0";
  // in regular intervals, check if the device becomes available - if yes, the user connected an ethernet hotspot device.
  while (!loopThreadStop){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::cout<<"Checking for USB tethering device\n";
	if(OHDFilesystemUtil::exists(connectedDevice)) {
	  std::cout << "Found USB tethering device\n";
	  break;
	}
  }
  // Configure the detected USB tether device (not sure if needed)
  OHDUtil::run_command("dhclient",{"usb0"});

  // now we find the IP of the connected device so we can forward video and more to it.
  // bit more complicated than needed.
  const auto ip_opt=OHDUtil::run_command_out("ip route show 0.0.0.0/0 dev usb0 | cut -d\\  -f3");
  if(ip_opt!=std::nullopt){
	auto ip=ip_opt.value();
	if(OHDUtil::endsWith(ip,"\n")){
	  ip.resize(ip.length()-1);
	}
	//int inet_pton(int af, const char *restrict src, void *restrict dst);
	if(!OHDUtil::is_valid_ip(ip)){
	  std::stringstream ss;
	  ss<<"USBTetherListener: not a valid IP:["<<ip<<"]\n";
	  std::cerr<<ss.str();
	  return;
	}
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
  while (!loopThreadStop){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::cout<<"Checking if USB tethering device disconnected\n";
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

void USBTetherListener::startLooping() {
  loopThreadStop=false;
  assert(loopThread== nullptr);
  loopThread=std::make_unique<std::thread>([this](){loopInfinite();});
}

void USBTetherListener::stopLooping() {
  loopThreadStop=true;
  if(loopThread->joinable()){
	loopThread->join();
  }
  loopThread.reset();
}

