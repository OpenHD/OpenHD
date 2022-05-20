//
// Created by consti10 on 19.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_

#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"
#include <thread>
#include <chrono>
#include <utility>
#include <mutex>


/**
 * USB hotspot (USB Tethering).
 * Since the USB tethering is always initiated by the user (when he switches USB Tethering on on his phone/tablet)
 * we don't need any settings or similar, and checking once every second barely uses any CPU resources.
 * This was created by translating the tether_functions.sh script from wifibroadcast-scripts into c++.
 * This class configures and forwards the connect and disconnect event(s) for a USB tethering device, such that
 * we can start/stop forwarding to the device's ip address.
 * Only supports one USB tethering device connected at the same time.
 * Also, assumes that the usb tethering device always shows up under /sys/class/net/usb0.
 */
class USBTetherListener{
 public:
  /**
   * Callback to be called when a new device has been connected/disconnected.
   * @param removed: true if the device has been removed (upper level should stop formwarding)
   * false if the device has been added (upper leved should start forwarding).
   */
  typedef std::function<void(bool removed,std::string ip)> IP_CALLBACK;
  /**
   * Creates a new USB tether listener which notifies the upper level with the IP address of a connected or
   * disconnected USB tether device.
   * @param ip_callback the callback to notify the upper level.
   */
  explicit USBTetherListener(IP_CALLBACK ip_callback):ip_callback(std::move(ip_callback)){}
  /**
   * Continuously checks for connected or disconnected USB tether devices.
   * Does not return as long as there is no fatal error, and blocks the calling thread.
   */
  [[noreturn]] void loopInfinite(){
	while (true){
	  connectOnce();
	}
  }
  /**
   * @return the valid ip address of the connected USB tether device if there is one. Empty if there is currently no device deteced.
   */
  [[nodiscard]] std::vector<std::string> getConnectedTetherIPs(){
	std::lock_guard<std::mutex> guard(device_ip_mutex);
	if(device_ip.empty()){
	  return {};
	}
	return std::vector<std::string>{device_ip};
  }
 private:
  const IP_CALLBACK ip_callback;
  // protects against simultaneous read/wrte of the device_ip variable.
  std::mutex device_ip_mutex;
  std::string device_ip;
  // write the device ip, protected by mutex.
  void setDeviceIp(std::string newDeviceIp){
	std::lock_guard<std::mutex> guard(device_ip_mutex);
	device_ip=std::move(newDeviceIp);
  }
  /**
   * @brief simple state-based method
   * 1) Wait until a tethering device is connected
   * 2) Configure and forward the IP address of the connected device
   * 3) Wait until the device disconnects
   * 4) forward the now disconnected IP address.
   * Nr. 3) might never become true during run time as long as the user does not disconnect his tethering device.
   */
  void connectOnce(){
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
	  setDeviceIp(ip_opt.value());
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
};

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_USBHOTSPOT_H_
