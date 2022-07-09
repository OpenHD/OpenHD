//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

#include <DWifiCards.h>

#include <utility>

OHDInterface::OHDInterface(const OHDProfile &profile1) : profile(profile1) {
  std::cout << "OHDInterface::OHDInterface()\n";
  //wifiCards = std::make_unique<WifiCards>(profile);

  //Find out which cards are connected first
  auto discovered_wifi_cards=DWifiCards::discover();
  // Find / create settings for each discovered card
  std::vector<std::shared_ptr<WifiCardHolder>> wifi_cards{};
  for(const auto& card:discovered_wifi_cards){
    wifi_cards.push_back(std::make_shared<WifiCardHolder>(card));
  }
  // now decide what to use the card(s) for
  std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards{};
  for(auto& card:wifi_cards){
    if(card->get_settings().use_for==WifiUseFor::MonitorMode){
      broadcast_cards.push_back(card);
    }
  }
  // We don't have at least one card for monitor mode, which is a hard requirement for OpenHD
  if(broadcast_cards.empty()){
    std::cerr<<"Cannot start ohd_interface, no wifi card for monitor mode\n";
    exit(1);
  }
  wbStreams=std::make_unique<WBStreams>(profile,broadcast_cards);

  // USB tethering - only on ground
  if(!profile.is_air){
    usbTetherListener=std::make_unique<USBTetherListener>([this](bool removed,std::string ip){
      if(removed){
        removeExternalDeviceIpForwarding(ip);
      }else{
        addExternalDeviceIpForwarding(ip);
      }
    });
    usbTetherListener->startLooping();
  }
  std::cout << "OHDInterface::created\n";
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
  if (wifiCards) {
    ss << wifiCards->createDebug();
  }
  if (wbStreams) {
    ss << wbStreams->createDebug();
  }
  //if(ethernet){
  //    ethernet->debug();
  //}
  ss<<"OHDInterface::createDebug:end\n";
  return ss.str();
}

void OHDInterface::addExternalDeviceIpForwarding(std::string ip) const {
  wbStreams->addExternalDeviceIpForwarding(ip);
}

void OHDInterface::removeExternalDeviceIpForwarding(std::string ip) const {
  wbStreams->removeExternalDeviceIpForwarding(ip);
}
