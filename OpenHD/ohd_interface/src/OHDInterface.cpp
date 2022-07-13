//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

#include <DWifiCards.h>

#include <utility>

OHDInterface::OHDInterface(OHDPlatform platform1,OHDProfile profile1,openhd::link_statistics::STATS_CALLBACK stats_callback) :
platform(platform1),profile(std::move(profile1)) {
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
    const std::string message_for_user="No WiFi card found, please reboot\n";
    LOGE<<message_for_user;
    // TODO reason what to do. We do not support dynamically adding wifi cards at run time, so somehow
    // we need to signal to the user that something is completely wrong. However, as an grund pi, we can still
    // run QOpenHD and OpenHD, just it will never connect to an air pi
    _error_blinker=std::make_unique<openhd::rpi::LEDBlinker>(message_for_user);
    // we just continue as nothing happened, but OHD won't be usable until a reboot.
    //exit(1);
  }else{
      wbStreams=std::make_unique<WBStreams>(profile,broadcast_cards,stats_callback);
  }

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
  const bool enable_wifi_hotspot=OHDFilesystemUtil::exists("/boot/enable_hotspot.txt")
                                   && platform.platform_type==PlatformType::RaspberryPi; // For now only supported on rpi
  if(enable_wifi_hotspot){
    if(optional_hotspot_card != nullptr){
      // Enable hotspot for this card
      //std::cout<<"TODO enable hotspot for: "<<optional_hotspot_card->_wifi_card.interface_name<<"\n";
      _wifi_hotspot=std::make_unique<WifiHotspot>(optional_hotspot_card->_wifi_card);
      _wifi_hotspot->start();
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
    if(wbStreams){
        wbStreams->addExternalDeviceIpForwarding(ip);
    }
}

void OHDInterface::removeExternalDeviceIpForwarding(std::string ip) const {
    if(wbStreams){
        wbStreams->removeExternalDeviceIpForwarding(ip);
    }
}
