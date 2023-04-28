//
// Created by consti10 on 02.05.22.
//

#include "ohd_interface.h"
#include "openhd_config.h"

#include <wifi_card_discovery.h>

#include <utility>

#include "wb_link.h"
#include "wb_link_settings.hpp"
#include "wifi_command_helper.h"

OHDInterface::OHDInterface(OHDPlatform platform1,OHDProfile profile1,std::shared_ptr<openhd::ActionHandler> opt_action_handler,bool continue_without_wb_card)
    : m_platform(platform1), m_profile(std::move(profile1)) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  m_external_devices_manager=std::make_shared<openhd::ExternalDeviceManager>();
  monitor_mode_cards={};
  opt_hotspot_card=std::nullopt;
  const auto config=openhd::load_config();
  if(config.WIFI_ENABLE_AUTODETECT){
    // We need to discover the connected cards and reason about their usage
    //Find out which cards are connected first
    auto connected_cards =DWifiCards::discover_connected_wifi_cards();
    // Issue on rpi with Atheros: For some reason, openhd is sometimes started before the card
    // finishes some initialization steps ?! and is therefore not discovered.
    // Change January 05, 23: We always wait for a card doing monitor mode unless a (developer) has specified the option to do otherwise
    // (which can be usefully for testing, but is not a behaviour we want when running on a user image)
    if(!continue_without_wb_card) {
      const auto begin = std::chrono::steady_clock::now();
      while (true) {
        if (DWifiCards::any_wifi_card_supporting_injection(connected_cards))
          break;
        const auto elapsed = std::chrono::steady_clock::now() - begin;
        if (elapsed > std::chrono::seconds(3)) {
          m_console->warn("Waiting for card supporting monitor mode+injection");
        } else {
          m_console->debug(
              "Waiting for card supporting monitor mode+injection");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        connected_cards = DWifiCards::discover_connected_wifi_cards();
        // after 10 seconds, we are happy with a card that only does monitor mode, aka is not known for injection
        if (elapsed > std::chrono::seconds(10)) {
          if (DWifiCards::any_wifi_card_supporting_monitor_mode(
                  connected_cards)) {
            m_console->warn("Using card without injection capabilities");
            break;
          }
        }
      }
    }
    // now decide what to use the card(s) for
    const auto evaluated=DWifiCards::process_and_evaluate_cards(connected_cards, m_platform, m_profile);
    monitor_mode_cards=evaluated.monitor_mode_cards;
    opt_hotspot_card=evaluated.hotspot_card;
  }else{
    // Much easier to do, no weird trying to figure out what to use the card(s) for
    auto processed=DWifiCards::find_cards_from_manual_file(config.WIFI_WB_LINK_CARDS,config.WIFI_WIFI_HOTSPOT_CARD);
    monitor_mode_cards=processed.monitor_mode_cards;
    opt_hotspot_card=processed.hotspot_card;
    if(m_profile.is_air && monitor_mode_cards.size()>1){
      m_console->warn("WB only supports one wifi card on air");
      monitor_mode_cards.resize(1);
    }
  }
  m_console->debug("monitor_mode card(s):{}",debug_cards(monitor_mode_cards));
  if(opt_hotspot_card.has_value()){
    m_console->debug("Hotspot card:{}",opt_hotspot_card.value().device_name);
  }else{
    m_console->debug("No WiFi hotspot card");
  }
  // We don't have at least one card for monitor mode, which means we cannot instantiate wb_link (no wifibroadcast connectivity at all)
  if(monitor_mode_cards.empty()){
    m_console->warn("Cannot start ohd_interface, no wifi card for monitor mode");
    const std::string message_for_user="No WiFi card found, please reboot";
    m_console->warn(message_for_user);
    // TODO reason what to do. We do not support dynamically adding wifi cards at run time, so somehow
    // we need to signal to the user that something is completely wrong. However, as an Ground pi, we can still
    // run QOpenHD and OpenHD, just it will never connect to an Air PI
    m_error_blinker=std::make_unique<openhd::LEDBlinker>(m_platform,message_for_user);
    // we just continue as nothing happened, but OHD won't have any wifibroadcast connectivity
    //exit(1);
  }else{
    m_wb_link =std::make_shared<WBLink>(m_profile, m_platform,monitor_mode_cards,opt_action_handler);
  }
  // Listen for external device(s) to connect - only on ground
  if(m_profile.is_ground()){
    // The USB tethering listener is always enabled on ground - it doesn't interfere with anything
    m_usb_tether_listener =std::make_unique<USBTetherListener>(m_external_devices_manager);
    // passive listening can also be enabled / disabled with reboots by the user
    if(m_nw_settings.get_settings().ethernet_nonhotspot_enable_auto_forwarding){
      m_ethernet_listener=std::make_unique<EthernetListener>(m_external_devices_manager,"eth0");
    }
  }
  if(openhd::nw_ethernet_card_manual_active(config)){
    // User explicitly told openhd to manage ethernet, which needs to be done if running on non-rpi platform
    // or when you want openhd to manage ethernet on the air unit
    const auto eth_card=config.NW_ETHERNET_CARD;
    m_console->debug("Managing manually specified card [{}]",eth_card);
    if(m_nw_settings.get_settings().ethernet_hotspot_enable){
      m_ethernet_hotspot = std::make_unique<EthernetHotspot>(m_external_devices_manager,eth_card);
    }else{
      EthernetHotspot::cleanup();
    }
  }else{
    // Default
    // RPI: manage ethernet on ground, which always shows up as eth0
    if(m_profile.is_ground() && m_platform.platform_type==PlatformType::RaspberryPi){
      if(m_nw_settings.get_settings().ethernet_hotspot_enable){
        m_ethernet_hotspot = std::make_unique<EthernetHotspot>(m_external_devices_manager,"eth0");
      }else{
        EthernetHotspot::cleanup();
      }
    }
  }
  // This way one could try and recover an air pi
  if(opt_hotspot_card.has_value()){
    const openhd::WifiSpace wb_frequency_space= (m_wb_link!= nullptr) ? m_wb_link->get_current_frequency_channel_space() : openhd::WifiSpace::G5_8;
    // OHD hotspot needs to know the wifibroadcast frequency - it is always on the opposite spectrum
    m_wifi_hotspot =std::make_unique<WifiHotspot>(opt_hotspot_card.value(),wb_frequency_space);
    if(m_nw_settings.get_settings().wifi_hotspot_enable){
      m_wifi_hotspot->set_enabled(true);
    }
  }
  m_console->debug("OHDInterface::created");
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
  if (m_wb_link) {
    ss << m_wb_link->createDebug();
  }
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
    auto cb_enable=[this](std::string,int value){
      if(!openhd::validate_yes_or_no(value))return false;
      m_nw_settings.unsafe_get_settings().wifi_hotspot_enable=value;
      m_nw_settings.persist();
      m_wifi_hotspot->set_enabled(m_nw_settings.get_settings().wifi_hotspot_enable);
      return true;
    };
    ret.push_back(openhd::Setting{"I_WIFI_HOTSPOT_E",openhd::IntSetting{
            m_nw_settings.get_settings().wifi_hotspot_enable,cb_enable}});
  }
  if(m_ethernet_hotspot){
    const auto settings=m_nw_settings.get_settings();
    auto cb_enable=[this](std::string,int value){
      if(!openhd::validate_yes_or_no(value))return false;
      m_nw_settings.unsafe_get_settings().ethernet_hotspot_enable=value;
      m_nw_settings.persist();
      // to apply, requires reboot !!
      return true;
    };
    ret.push_back(openhd::Setting{"I_ETH_HOTSPOT_E",openhd::IntSetting{settings.ethernet_hotspot_enable,cb_enable}});
  }
  if(true){
    const auto settings=m_nw_settings.get_settings();
    auto cb_enable=[this](std::string,int value){
      if(!openhd::validate_yes_or_no(value))return false;
      m_nw_settings.unsafe_get_settings().ethernet_nonhotspot_enable_auto_forwarding=value;
      m_nw_settings.persist();
      // to apply, requires reboot !!
      return true;
    };
    ret.push_back(openhd::Setting{"ETH_PASSIVE_F",openhd::IntSetting{settings.ethernet_nonhotspot_enable_auto_forwarding,cb_enable}});
  }
  if(monitor_mode_cards.empty()){
    auto setting=openhd::create_read_only_string(fmt::format("WIFI_CARD{}",0), "NOTFOUND");
    ret.emplace_back(setting);
  }else{
    for(int i=0;i<monitor_mode_cards.size();i++){
      auto setting=openhd::create_read_only_string(fmt::format("WIFI_CARD{}",i), wifi_card_type_to_string(monitor_mode_cards[i].type));
      ret.emplace_back(setting);
    }
  }
  if(opt_hotspot_card){
    auto setting=openhd::create_read_only_string(fmt::format("HOTSPOT_CARD"), wifi_card_type_to_string(opt_hotspot_card.value().type));
    ret.emplace_back(setting);
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
