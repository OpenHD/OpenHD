//
// Created by consti10 on 21.05.22.
//

#include "USBTetherListener.h"
#include <arpa/inet.h>

#include <utility>


void USBTetherListener::setExternalDeviceLocked(openhd::ExternalDevice device) {
  std::lock_guard<std::mutex> guard(device_ip_mutex);
  _externalDevice=std::move(device);
}

void USBTetherListener::loopInfinite() {
  while (!loopThreadStop){
	connectOnce();
  }
}

// from https://stackoverflow.com/questions/3339200/get-string-between-2-strings
static std::string string_in_between(const std::string& start,const std::string& end,const std::string& value){
  std::regex base_regex(start + "(.*)" + end);
  std::smatch base_match;
  std::string matched;
  if (std::regex_search(value, base_match, base_regex)) {
	// The first sub_match is the whole string; the next
	// sub_match is the first parenthesized expression.
	if (base_match.size() == 2) {
	  matched = base_match[1].str();
	}
  }
  std::stringstream ss;
  ss<<"Given:{"<<value<<"}\n";
  ss<<"Result:{"<<matched<<"}\n";
  std::cout <<ss.str();
  return matched;
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
	std::cerr<<"USBHotspot run command out no result\n";
	return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  const auto ip_external_device= string_in_between("default via "," proto",run_command_result);
  const auto ip_self_network= string_in_between("src "," metric",run_command_result);

  if(!OHDUtil::is_valid_ip(ip_external_device)){
	std::stringstream ss;
	ss<<"USBTetherListener: not a valid IP:["<<ip_external_device<<"]\n";
	std::cerr<<ss.str();
	return;
  }

  setExternalDeviceLocked(openhd::ExternalDevice{ip_self_network,ip_external_device});
  std::cout<<"USBTetherListener::found device:"<<_externalDevice.to_string()<<"\n";
  if(_external_device_callback){
	_external_device_callback(_externalDevice, true);
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
	_external_device_callback(_externalDevice, false);
  }
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

