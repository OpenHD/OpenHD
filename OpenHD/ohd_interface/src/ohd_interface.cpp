//
// Created by consti10 on 02.05.22.
//

#include "ohd_interface.h"

#include <wifi_card_discovery.h>

#include <utility>

#include "WBStreamsSettings.hpp"
#include "wifi_command_helper.hpp"

OHDInterface::OHDInterface(OHDPlatform platform1,OHDProfile profile1,std::shared_ptr<openhd::ActionHandler> opt_action_handler) :
platform(platform1),profile(std::move(profile1)) {
  m_console = openhd::log::create_or_get("interface");
  assert(m_console);
  m_console->debug("OHDInterface::OHDInterface()");
  //wifiCards = std::make_unique<WifiCards>(profile);
  //Find out which cards are connected first
  auto discovered_wifi_cards=DWifiCards::discover();
  // Issue on rpi with Atheros: For some reason, openhd is sometime started before the card
  // finishes some initialization steps ?! and is therefore not discovered.
  // On a rpi, we block for up to 10 seconds here until we have at least one wifi card that does wifibroadcast
  // Note that we cannot just block until we have one, starting openhd anyways without a wifi card is an essential
  // feature for testing.
  if(platform.platform_type==PlatformType::RaspberryPi){
    const auto begin=std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now()-begin<std::chrono::seconds(10)){
      if(DWifiCards::any_wifi_card_supporting_monitor(discovered_wifi_cards))break;
      m_console->debug("rpi-waiting up to 10 seconds until at least one wifi card supporting monitor mode is found");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      discovered_wifi_cards=DWifiCards::discover();
    }
  }
  // Just to be sure, turn off all detected wifi cards here - they will be re-enabled when needed anyways
  // I don't do that on x86 though, since it would disrupt development
  /*if(platform.platform_type!=PlatformType::PC){
    for(const auto& card:discovered_wifi_cards){
      WifiCardCommandHelper::set_card_state(card, false);
    }
  }*/
  // Find / create settings for each discovered card
  std::vector<std::shared_ptr<WifiCardHolder>> wifi_cards{};
  for(const auto& card:discovered_wifi_cards){
    wifi_cards.push_back(std::make_shared<WifiCardHolder>(card));
    std::stringstream message;
    message << "OHDInterface:: found wifi card: (" << wifi_card_type_to_string(card.type) << ") interface: " << card.interface_name;
    m_console->debug(message.str());
  }
  // now check if any settings are messed up
  for(const auto& card : wifi_cards){
    if (card->get_settings().use_for == WifiUseFor::MonitorMode && !card->_wifi_card.supports_injection) {
      m_console->warn("Cannot use monitor mode on card that cannot inject");
    }
  }
  m_interface_settings_holder =std::make_shared<openhd::OHDInterfaceSettingsHolder>();
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
    m_console->warn("Cannot start ohd_interface, no wifi card for monitor mode");
    const std::string message_for_user="No WiFi card found, please reboot";
    m_console->warn(message_for_user);
    // TODO reason what to do. We do not support dynamically adding wifi cards at run time, so somehow
    // we need to signal to the user that something is completely wrong. However, as an grund pi, we can still
    // run QOpenHD and OpenHD, just it will never connect to an air pi
    m_error_blinker=std::make_unique<openhd::LEDBlinker>(platform,message_for_user);
    // we just continue as nothing happened, but OHD won't have any wifibroadcast connectivity
    //exit(1);
  }else{
    m_wb_streams =std::make_unique<WBStreams>(profile,platform,broadcast_cards);
  }
  // USB tethering - only on ground
  if(!profile.is_air){
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
  const bool enable_hotspot_file_exists=OHDFilesystemUtil::exists("/boot/enable_wifi_hotspot.txt");
  if(!m_interface_settings_holder->unsafe_get_settings().enable_wifi_hotspot && enable_hotspot_file_exists){
    m_console->info("Changing enable wifi hotspot to true due to file forcing it");
    m_interface_settings_holder->unsafe_get_settings().enable_wifi_hotspot= true;
    m_interface_settings_holder->persist();
  }
  if(m_interface_settings_holder->get_settings().enable_wifi_hotspot && optional_hotspot_card==nullptr){
    m_console->warn("Wifi hotspot enabled, but no card to start it with found");
    // we cannot do wifi hotspot
    m_interface_settings_holder->unsafe_get_settings().enable_wifi_hotspot=false;
    m_interface_settings_holder->persist();
  }
  // wifi hotspot - normally only on ground, but for now on both
  if(optional_hotspot_card){
    m_console->debug("Optional hotspot card exists");
    // create it when there is a card - note that this does not enable the hotspot yet.
    m_wifi_hotspot =std::make_unique<WifiHotspot>(optional_hotspot_card->_wifi_card);
    if(m_interface_settings_holder->get_settings().enable_wifi_hotspot){
      m_wifi_hotspot->start_async();
    }else{
      // Make sure the rpi internal wifi is disabled when hotspot is disabled to not interfere
      WifiCardCommandHelper::set_card_state(optional_hotspot_card->_wifi_card, false);
    }
  }else{
    m_console->debug("Optional hotspot card does not exist");
  }
  m_console->debug("OHDInterface::created");
}

std::string OHDInterface::createDebug() const {
  std::stringstream ss;
  ss<<"OHDInterface::createDebug:begin\n";
  if (m_wb_streams) {
    ss << m_wb_streams->createDebug();
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
  if(m_wb_streams){
    m_wb_streams->addExternalDeviceIpForwardingVideoOnly(external_device.external_device_ip);
  }
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  if(m_external_device_callback){
    m_external_device_callback(external_device, true);
  }
}

void OHDInterface::removeExternalDeviceIpForwarding(const openhd::ExternalDevice& external_device){
  if(m_wb_streams){
    m_wb_streams->removeExternalDeviceIpForwardingVideoOnly(external_device.external_device_ip);
  }
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  if(m_external_device_callback){
    m_external_device_callback(external_device, false);
  }
}

void OHDInterface::set_stats_callback(openhd::link_statistics::STATS_CALLBACK stats_callback) const {
  if(m_wb_streams){
    m_wb_streams->set_callback(std::move(stats_callback));
  }else{
    m_console->warn("Cannot ste stats callback, no wb streams instance");
  }
}

static constexpr auto OHD_INTERFACE_ENABLE_WIFI_HOTSPOT="I_WIFI_HOTSPOT_E";

std::vector<openhd::Setting> OHDInterface::get_all_settings(){
  std::vector<openhd::Setting> ret;
  if(m_wb_streams){
    auto settings= m_wb_streams->get_all_settings();
    for(const auto& setting:settings){
      ret.emplace_back(setting);
    }
    //ret.insert(ret.end(),settings.begin(),settings.end());
  }
  if(m_wifi_hotspot != nullptr){
    // we can disable / enable wifi hotspot.
    int enabled=
        m_interface_settings_holder->get_settings().enable_wifi_hotspot;
    auto change_wifi_hotspot=openhd::IntSetting{enabled,[this](std::string,int value){
                                                    // temporarily disable wifi hotspot (do not allow an user to turn it on)
                                                    // until we've fixed the OS
                                                    return false;
                                                    if(value== 0 || value== 1){
                                                      const bool enableX=value;
                                                      if(enableX){
                                                        m_wifi_hotspot
                                                            ->start_async();
                                                      }else{
                                                        m_wifi_hotspot
                                                            ->stop_async();
                                                      }
                                                      return true;
                                                    }
                                                    return false;
                                                  }};
    ret.emplace_back(openhd::Setting{OHD_INTERFACE_ENABLE_WIFI_HOTSPOT,change_wifi_hotspot});
  }
  if(!profile.is_air){
    //openhd::testing::append_dummy_int_and_string(ret);
  }
  openhd::validate_provided_ids(ret);
  return ret;
}

void OHDInterface::set_external_device_callback(openhd::EXTERNAL_DEVICE_CALLBACK cb) {
  std::lock_guard<std::mutex> guard(m_external_device_callback_mutex);
  m_external_device_callback =std::move(cb);
}

void OHDInterface::restart_wb_streams_async() {
  if(m_wb_streams){
    m_wb_streams->restart_async(std::chrono::seconds(2));
  }
}
void OHDInterface::print_internal_fec_optimization_method() {
  print_optimization_method();
}

void OHDInterface::set_video_codec(int codec) {
  if(m_wb_streams){
    m_wb_streams->set_video_codec(codec);
  }
}
