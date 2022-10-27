//
// Created by consti10 on 21.05.22.
//

#include "USBTetherListener.h"

#include <arpa/inet.h>

#include "openhd-spdlog.hpp"
#include <utility>

USBTetherListener::USBTetherListener(openhd::EXTERNAL_DEVICE_CALLBACK external_device_callback) :
_external_device_callback(std::move(external_device_callback)){

}

USBTetherListener::~USBTetherListener() {
  stopLooping();
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
  //OHDUtil::run_command("dhclient",{"usb0"});
  // Ubuntu pc:
  //ip route list dev usb0
  //default via 192.168.18.229 proto dhcp metric 101
  //192.168.18.0/24 proto kernel scope link src 192.168.18.155 metric 101

  // now we find the IP of the connected device so we can forward video and more to it.
  // bit more complicated than needed.
  //const auto run_command_result_opt=OHDUtil::run_command_out("ip route show 0.0.0.0/0 dev usb0 | cut -d\\  -f3");
  const auto run_command_result_opt=OHDUtil::run_command_out("ip route list dev usb0");
  if(run_command_result_opt==std::nullopt){
	openhd::loggers::get_default()->warn("USBHotspot run command out no result");
	return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  const auto ip_external_device= OHDUtil::string_in_between("default via "," proto",run_command_result);
  const auto ip_self_network= OHDUtil::string_in_between("src "," metric",run_command_result);

  const auto external_device=openhd::ExternalDevice{ip_self_network,ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if(!external_device.is_valid()){
	std::stringstream ss;
	ss<<"USBTetherListener: "<<external_device.to_string()<<" not valid\n";
	std::cerr<<ss.str();
	return;
  }

  std::cout<<"USBTetherListener::found device:"<<external_device.to_string()<<"\n";
  if(_external_device_callback){
	_external_device_callback(external_device, true);
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
  if(_external_device_callback){
	_external_device_callback(external_device, false);
  }
}


