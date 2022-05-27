//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

#include <utility>

OHDInterface::OHDInterface(const OHDProfile &profile) : profile(profile) {
  std::cout << "OHDInterface::OHDInterface()\n";
  wifiCards = std::make_unique<WifiCards>(profile);
  //ethernet=std::make_unique<EthernetCards>(is_air, unit_id);
  streams = std::make_unique<WBStreams>(profile);
  try {
	wifiCards->configure();
	//ethernet->configure();
	streams->set_broadcast_card_names(wifiCards->get_broadcast_card_names());
	streams->configure();
	//
	usbTetherListener=std::make_unique<USBTetherListener>([this](bool removed,std::string ip){
	  if(removed){
		removeExternalDeviceIpForwarding(ip);
	  }else{
		addExternalDeviceIpForwarding(ip);
	  }
	});
	usbTetherListener->startLooping();
  } catch (std::exception &ex) {
	std::cerr << "Error: " << ex.what() << std::endl;
	exit(1);
  } catch (...) {
	std::cerr << "Unknown exception occurred" << std::endl;
	exit(1);
  }
  std::cout << "OHDInterface::created\n";
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
  if (wifiCards) {
	ss << wifiCards->createDebug();
  }
  if (streams) {
	ss<<streams->createDebug();
  }
  //if(ethernet){
  //    ethernet->debug();
  //}
  ss<<"OHDInterface::createDebug:end\n";
  return ss.str();
}

void OHDInterface::addExternalDeviceIpForwarding(std::string ip) {
  streams->addExternalDeviceIpForwarding(std::move(ip));
}

void OHDInterface::removeExternalDeviceIpForwarding(std::string ip) {
	streams->removeExternalDeviceIpForwarding(std::move(ip));
}
