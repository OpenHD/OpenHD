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
    std::stringstream message;
    message << "OHDInterface:: found wifi card: (" << wifi_card_type_to_string(card.type) << ") interface: " << card.interface_name << std::endl;
    std::cout<<message.str();
  }
  // now check if any settings are messed up
  for(const auto& card : wifi_cards){
    if (card->get_settings().use_for == WifiUseFor::MonitorMode && !card->_wifi_card.supports_injection) {
      std::cerr << "Cannot use monitor mode on card that cannot inject\n";
    }
  }
  // now decide what to use the card(s) for
  std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards{};
  std::shared_ptr<WifiCardHolder> optional_hotspot_card=nullptr;
  for(auto& card:wifi_cards){
    if(card->get_settings().use_for==WifiUseFor::MonitorMode){
      broadcast_cards.push_back(card);
    }else if(card->_wifi_card.supports_hotspot){
      if(optional_hotspot_card== nullptr){
        optional_hotspot_card=card;
      }
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
  // wifi hotspot - normally only on ground, but for now on both
  const bool enable_wifi_hotspot=OHDFilesystemUtil::exists("/boot/enable_hotspot.txt");
  if(enable_wifi_hotspot){
    if(optional_hotspot_card != nullptr){
      // Enable hotspot for this card
      std::cout<<"TODO enable hotspot for: "<<optional_hotspot_card->_wifi_card.interface_name<<"\n";
    }
  }

  std::cout << "OHDInterface::created\n";
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
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
