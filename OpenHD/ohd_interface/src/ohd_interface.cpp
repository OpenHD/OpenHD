//
// Created by consti10 on 02.05.22.
//

#include "ohd_interface.h"

#include <wifi_card_discovery.h>

#include <utility>

#include "wb_link_settings.hpp"
#include "wifi_command_helper.h"

#include "manually_defined_cards.h"

OHDInterface::OHDInterface(OHDPlatform platform1,OHDProfile profile1,std::shared_ptr<openhd::ActionHandler> opt_action_handler) : m_platform(platform1), m_profile(std::move(profile1)) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  openhd::write_manual_cards_template();
  //wifiCards = std::make_unique<WifiCards>(profile);
  //Find out which cards are connected first
  auto discovered_wifi_cards=DWifiCards::discover();
  // Issue on rpi with Atheros: For some reason, openhd is sometime started before the card
  // finishes some initialization steps ?! and is therefore not discovered.
  // On a rpi, we block for up to 10 seconds here until we have at least one wifi card that does wifibroadcast
  // Note that we cannot just block until we have one, starting openhd anyways without a wifi card is an essential
  // feature for testing.
  if(m_platform.platform_type==PlatformType::RaspberryPi){
    const auto begin=std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now()-begin<std::chrono::seconds(10)){
      if(DWifiCards::any_wifi_card_supporting_injection(discovered_wifi_cards))break;
      m_console->debug("rpi-waiting up to 10 seconds until at least one wifi card supporting monitor mode is found");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      discovered_wifi_cards=DWifiCards::discover();
    }
  }
  // now decide what to use the card(s) for
  const auto evaluated=DWifiCards::process_and_evaluate_cards(discovered_wifi_cards, m_platform, m_profile);
  const auto broadcast_cards=evaluated.monitor_mode_cards;
  const auto optional_hotspot_card=evaluated.hotspot_card;
  m_console->debug("Broadcast card(s):{}",debug_cards(broadcast_cards));
  if(optional_hotspot_card.has_value()){
    m_console->debug("Hotspot card:{}",optional_hotspot_card.value().device_name);
  }else{
    m_console->debug("Not WiFi hotspot card");
  }
  // We don't have at least one card for monitor mode, which is a hard requirement for OpenHD
  if(broadcast_cards.empty()){
    m_console->warn("Cannot start ohd_interface, no wifi card for monitor mode");
    const std::string message_for_user="No WiFi card found, please reboot";
    m_console->warn(message_for_user);
    // TODO reason what to do. We do not support dynamically adding wifi cards at run time, so somehow
    // we need to signal to the user that something is completely wrong. However, as an grund pi, we can still
    // run QOpenHD and OpenHD, just it will never connect to an air pi
    m_error_blinker=std::make_unique<openhd::LEDBlinker>(m_platform,message_for_user);
    // we just continue as nothing happened, but OHD won't have any wifibroadcast connectivity
    //exit(1);
  }else{
    m_wb_link =std::make_shared<WBLink>(m_profile, m_platform,broadcast_cards,opt_action_handler);
  }
  // USB tethering - only on ground
  if(!m_profile.is_air){
    m_usb_tether_listener =std::make_unique<USBTetherListener>([this](openhd::ExternalDevice external_device,bool connected){
      if(connected){
        addExternalDeviceIpForwarding(external_device);
      }else{
        removeExternalDeviceIpForwarding(external_device);
      }
    });
    m_usb_tether_listener->startLooping();
  }
  // This way one could try and recover an air pi
  if(optional_hotspot_card.has_value()){
    m_wifi_hotspot =std::make_unique<WifiHotspot>(optional_hotspot_card.value());
  }
  m_console->debug("OHDInterface::created");
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
  if (m_wb_link) {
    ss << m_wb_link->createDebug();
  }
  //if(ethernet){
  //    ethernet->debug();
  //}
  ss<<"OHDInterface::createDebug:end\n";
  return ss.str();
}

void OHDInterface::addExternalDeviceIpForwarding(const openhd::ExternalDevice& external_device){
  // video we can directly forward to the external device - but note that
  // telemetry first needs to go through the ohd_telemetry module, and therefore is handled
  // seperately ( a bit hacky, but no real way around if we want to keep the module separation)
  if(m_wb_link){
    m_wb_link->addExternalDeviceIpForwardingVideoOnly(external_device.external_device_ip);
  }
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  if(m_external_device_callback){
    m_external_device_callback(external_device, true);
  }
}

void OHDInterface::removeExternalDeviceIpForwarding(const openhd::ExternalDevice& external_device){
  if(m_wb_link){
    m_wb_link->removeExternalDeviceIpForwardingVideoOnly(external_device.external_device_ip);
  }
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  if(m_external_device_callback){
    m_external_device_callback(external_device, false);
  }
}

static constexpr auto OHD_INTERFACE_ENABLE_WIFI_HOTSPOT="I_WIFI_HOTSPOT_E";

std::vector<openhd::Setting> OHDInterface::get_all_settings(){
  std::vector<openhd::Setting> ret;
  if(m_wb_link){
    auto settings= m_wb_link->get_all_settings();
    for(const auto& setting:settings){
      ret.emplace_back(setting);
    }
    //ret.insert(ret.end(),settings.begin(),settings.end());
  }
  if(m_wifi_hotspot != nullptr){
    // TODO wifi hotspot manages its own settings
  }
  if(!m_profile.is_air){
    //openhd::testing::append_dummy_int_and_string(ret);
  }
  openhd::validate_provided_ids(ret);
  return ret;
}

void OHDInterface::set_external_device_callback(openhd::EXTERNAL_DEVICE_CALLBACK cb) {
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  m_external_device_callback =std::move(cb);
}

void OHDInterface::print_internal_fec_optimization_method() {
  print_optimization_method();
}

std::shared_ptr<OHDLink> OHDInterface::get_link_handle() {
  if(m_wb_link){
    return m_wb_link;
  }
  return nullptr;
}

