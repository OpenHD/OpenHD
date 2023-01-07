//
// Created by consti10 on 02.05.22.
//

#include "ohd_interface.h"

#include <wifi_card_discovery.h>

#include <utility>

#include "wb_link_settings.hpp"
#include "wifi_command_helper.h"

#include "manually_defined_cards.h"

OHDInterface::OHDInterface(OHDPlatform platform1,OHDProfile profile1,std::shared_ptr<openhd::ActionHandler> opt_action_handler,bool continue_without_wb_card)
    : m_platform(platform1), m_profile(std::move(profile1)) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  openhd::write_manual_cards_template();
  m_external_devices_manager=std::make_shared<openhd::ExternalDeviceManager>();
  //Find out which cards are connected first
  auto connected_cards =DWifiCards::discover_connected_wifi_cards();
  // Issue on rpi with Atheros: For some reason, openhd is sometimes started before the card
  // finishes some initialization steps ?! and is therefore not discovered.
  // Change January 05, 23: We always wait for a card doing monitor mode unless a (developer) has specified the option to do otherwise
  // (which can be usefully for testing, but is not a behaviour we want when running on a user image)
  if(!continue_without_wb_card && !openhd::manually_defined_cards_file_exists()){
    m_console->debug("Waiting for card ");
    const auto begin=std::chrono::steady_clock::now();
    while (true){
      if(DWifiCards::any_wifi_card_supporting_injection(connected_cards))break;
      const auto elapsed=std::chrono::steady_clock::now()-begin;
      if(elapsed>std::chrono::seconds(3)){
        m_console->warn("Waiting for card supporting monitor mode+injection");
      }else{
        m_console->debug("Waiting for card supporting monitor mode+injection");
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
      connected_cards =DWifiCards::discover_connected_wifi_cards();
      // after 10 seconds, we are happy with a card that only does monitor mode, aka is not known for injection
      if(elapsed>std::chrono::seconds(10)){
        if(DWifiCards::any_wifi_card_supporting_monitor_mode(connected_cards)){
          m_console->warn("Using card without injection capabilities");
          break;
        }
      }
    }
  }
  // now decide what to use the card(s) for
  const auto evaluated=DWifiCards::process_and_evaluate_cards(
      connected_cards, m_platform, m_profile);
  const auto broadcast_cards=evaluated.monitor_mode_cards;
  const auto optional_hotspot_card=evaluated.hotspot_card;
  m_console->debug("Broadcast card(s):{}",debug_cards(broadcast_cards));
  if(optional_hotspot_card.has_value()){
    m_console->debug("Hotspot card:{}",optional_hotspot_card.value().device_name);
  }else{
    m_console->debug("No WiFi hotspot card");
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
  // Listen for external device(s) to connect - only on ground
  if(m_profile.is_ground()){
    //m_usb_tether_listener =std::make_unique<USBTetherListener>(m_external_devices_manager);
    //m_ethernet_listener = std::make_unique<EthernetListener>(m_external_devices_manager);
    //
    //m_ethernet_hotspot = std::make_unique<EthernetHotspot>("eth0");
  }
  // This way one could try and recover an air pi
  if(optional_hotspot_card.has_value()){
    openhd::Space wb_frequency_space= (m_wb_link!= nullptr) ? m_wb_link->get_current_frequency_channel_space() : openhd::Space::G5_8;
    m_wifi_hotspot =std::make_unique<WifiHotspot>(optional_hotspot_card.value(),wb_frequency_space);
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

std::vector<openhd::Setting> OHDInterface::get_all_settings(){
  std::vector<openhd::Setting> ret;
  if(m_wb_link){
    auto settings= m_wb_link->get_all_settings();
    OHDUtil::vec_append(ret,settings);
  }
  if(m_wifi_hotspot != nullptr){
    auto settings= m_wifi_hotspot->get_all_settings();
    OHDUtil::vec_append(ret,settings);
  }
  if(m_ethernet_hotspot){
    auto settings = m_ethernet_hotspot->get_all_settings();
    OHDUtil::vec_append(ret,settings);
  }
  if(!m_profile.is_air){
    //openhd::testing::append_dummy_int_and_string(ret);
  }
  openhd::validate_provided_ids(ret);
  return ret;
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

std::shared_ptr<openhd::ExternalDeviceManager> OHDInterface::get_ext_devices_manager() {
  return m_external_devices_manager;
}
