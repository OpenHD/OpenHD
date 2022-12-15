#include "wifi_card_discovery.h"

#include <iostream>
#include <regex>

#include "openhd-spdlog.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"
#include "wifi_card.hpp"
#include "wifi_command_helper.h"
#include "manually_defined_cards.h"

static WiFiCardType driver_to_wifi_card_type(const std::string &driver_name) {
  if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("ath9k_htc")) != std::string::npos) {
    return WiFiCardType::Atheros9khtc;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("ath9k")) != std::string::npos) {
    return WiFiCardType::Atheros9k;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("rt2800usb")) != std::string::npos) {
    return WiFiCardType::Ralink;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("iwlwifi")) != std::string::npos) {
    return WiFiCardType::Intel;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("brcmfmac")) != std::string::npos) {
    return WiFiCardType::Broadcom;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("88xxau")) != std::string::npos) {
    return WiFiCardType::Realtek8812au;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("8812au")) != std::string::npos) {
    return WiFiCardType::Realtek8812au;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("88x2bu")) != std::string::npos) {
    return WiFiCardType::Realtek88x2bu;
  } else if (OHDUtil::to_uppercase(driver_name).find(OHDUtil::to_uppercase("8188eu")) != std::string::npos) {
    return WiFiCardType::Realtek8188eu;
  }
  return WiFiCardType::Unknown;
}

static std::vector<uint32_t> supported_frequencies(const std::string& device,bool check_2g){
  auto channels_to_try=check_2g ? openhd::get_channels_2G() : openhd::get_channels_5G();
  const auto tmp=openhd::get_all_channel_frequencies(channels_to_try);
  auto supported_frequencies=wifi::commandhelper::
      iw_get_supported_frequencies(device,openhd::get_all_channel_frequencies(channels_to_try));
  return supported_frequencies;
}

bool DWifiCards::is_known_for_injection(const WiFiCardType& type) {
  bool supports=false;
  switch (type) {
    case WiFiCardType::Unknown:
      supports= false;
      break;
    case WiFiCardType::Realtek8812au:
    case WiFiCardType::Realtek8814au:
    case WiFiCardType::Realtek88x2bu:
    case WiFiCardType::Realtek8188eu:
    case WiFiCardType::Atheros9khtc:
    case WiFiCardType::Atheros9k:
    case WiFiCardType::Ralink:
      supports= true;
      break;
    case WiFiCardType::Intel:
    case WiFiCardType::Broadcom:
    default:
      supports= false;
      break;
  }
  return supports;
}


std::optional<WiFiCard> DWifiCards::fill_linux_wifi_card_identifiers(const std::string& interface_name) {
  // get the driver name for this card
  const auto filename_device_uevent=fmt::format("/sys/class/net/{}/device/uevent",interface_name);
  const auto device_uevent_content=OHDFilesystemUtil::read_file(filename_device_uevent);
  const std::regex driver_regex{"DRIVER=([\\w]+)"};
  std::smatch result;
  if (!std::regex_search(device_uevent_content, result, driver_regex)) {
    openhd::log::get_default()->warn("no result driver regex [{}]",device_uevent_content);
    return std::nullopt;
  }
  if (result.size() != 2) {
    openhd::log::get_default()->warn("result doesnt match");
    return std::nullopt;
  }
  const std::string driver_name = result[1];

  WiFiCard card{};
  card.device_name = interface_name;
  card.driver_name=driver_name;
  card.type = driver_to_wifi_card_type(driver_name);

  // get the phy index for this card
  const auto filename_phy_index=fmt::format("/sys/class/net/{}/phy80211/index",card.device_name);
  const auto phy_val=OHDFilesystemUtil::read_file(filename_phy_index);
  const auto opt_phy_phy80211_index=OHDUtil::string_to_int(phy_val);
  if(!opt_phy_phy80211_index.has_value()){
    openhd::log::get_default()->warn("Cannot find phy index for card {}",interface_name);
    return std::nullopt;
  }
  card.phy80211_index =opt_phy_phy80211_index.value();

  // get the mac address for this card
  const auto filename_mac_address=fmt::format("/sys/class/net/{}/address",card.device_name);
  auto mac=OHDFilesystemUtil::read_file(filename_mac_address);
  if(mac.empty()){
    openhd::log::get_default()->warn("Cannot find mac for card {}",interface_name);
    return std::nullopt;
  }
  OHDUtil::rtrim(mac);
  card.mac = mac;
  // Here we are done with the unique identifiers
  assert(!card.device_name.empty());
  assert(!card.mac.empty());
  assert(card.phy80211_index !=-1);
  assert(!card.driver_name.empty());
  return card;
}

std::vector<WiFiCard> DWifiCards::discover_connected_wifi_cards() {
  openhd::log::get_default()->trace("WiFi::discover_connected_wifi_cards");
  std::vector<WiFiCard> wifi_cards{};
  // Find wifi cards, excluding specific kinds of interfaces.
  const std::vector<std::string> excluded_interfaces = {
      "usb",
      "lo",
      "eth",
      "enp2s0" // Consti10 added
  };
  const auto netFilenames=OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory("/sys/class/net");
  for(const auto& filename:netFilenames){
    auto excluded = false;
    for (const auto &excluded_interface: excluded_interfaces) {
      if (filename.find(excluded_interface)!=std::string::npos) {
        excluded = true;
        break;
      }
    }
    if (!excluded) {
      auto card_opt= process_card(filename);
      if(card_opt.has_value()){
        wifi_cards.push_back(card_opt.value());
      }
    }
  }
  openhd::log::get_default()->trace("WiFi::discover_connected_wifi_cards done, n cards: {}",wifi_cards.size());
  write_wificards_manifest(wifi_cards);
  return wifi_cards;
}


std::optional<WiFiCard> DWifiCards::process_card(const std::string &interface_name) {
  auto card_opt= fill_linux_wifi_card_identifiers(interface_name);
  if(!card_opt.has_value()){
    return std::nullopt;
  }
  WiFiCard card=card_opt.value();

  // This reported value is right in most cases
  /*const auto supported_freq= wifi::commandhelper::iw_get_supported_frequency_bands(card.device_name);
  card.xx_supports_2ghz=supported_freq.supports_any_2G;
  card.xx_supports_5ghz=supported_freq.supports_any_5G;*/
  // but we now also have a method to figure out all the supported channels
  card.supported_frequencies_2G=supported_frequencies(card.device_name, true);
  card.supported_frequencies_5G=supported_frequencies(card.device_name, false);

  // Note that this does not necessarily mean this info is right/complete
  // a card might report a specific channel but then since monitor mode is so hack not support the channel in monitor mode
  card.supports_monitor_mode= wifi::commandhelper::iw_supports_monitor_mode(card.phy80211_index);
  card.supports_injection= is_known_for_injection(card.type);

  openhd::log::get_default()->debug("Card {} reports driver:{} supports_2GHz:{} supports_5GHz:{} supports_monitor_mode:{} supports_injection:{}",
                                    card.device_name,card.driver_name,card.supports_2GHz(),card.supports_5GHz(),
                                    card.supports_monitor_mode,card.supports_injection);

  // temporary,hacky, only hotspot on rpi integrated wifi
  if(card.type==WiFiCardType::Broadcom){
    card.supports_hotspot= true;
  }

  if(openhd::ignore::should_be_ignored_interface(card.device_name)
      || openhd::ignore::should_be_ignored_mac(card.mac)){
    openhd::log::get_default()->info("Ignoring card {} since whitelisted by developer",card.device_name);
    return std::nullopt;
  }
  return card;
}

bool DWifiCards::any_wifi_card_supporting_injection(const std::vector<WiFiCard>& cards) {
  for(const auto& card:cards){
    if(card.supports_injection)return true;
  }
  return false;
}

DWifiCards::ProcessedWifiCards DWifiCards::process_and_evaluate_cards(
    const std::vector<WiFiCard>& discovered_cards,const OHDPlatform& platform,const OHDProfile& profile){

  // manually overwriting the automatic assignment of cards depending on their capabilities can result in bugs / undefined behaviour
  if(openhd::manually_defined_cards_file_exists()){
      const auto manual_cards=openhd::get_manually_defined_cards_from_file(openhd::FILE_PATH_MANUALLY_DEFINED_CARDS);
      std::vector<WiFiCard> monitor_mode_cards{};
      std::optional<WiFiCard> opt_hotspot_card =std::nullopt;
      for(const auto& manual_card : manual_cards){
        auto card_opt= process_card(manual_card.interface_name);
        if(!card_opt){
          throw std::runtime_error(fmt::format("Card {} does not exist on this system",manual_card.interface_name));
        }
        if(manual_card.usage==WifiUseFor::MonitorMode){
          monitor_mode_cards.push_back(card_opt.value());
        }else if(manual_card.usage==WifiUseFor::Hotspot){
          WiFiCard card=card_opt.value();
          opt_hotspot_card =card;
        }
      }
      return {monitor_mode_cards, opt_hotspot_card};
  }
  // We need to figure out what's the best usage for the card(s) connected to the system based on their capabilities.
  std::vector<WiFiCard> monitor_mode_cards{};
  std::optional<WiFiCard> hotspot_card=std::nullopt;
  // Default simple approach: if a card supports injection, use it for monitor mode
  // otherwise, use it for hotspot
  for(const auto& card:discovered_cards){
    if(card.supports_injection){
      monitor_mode_cards.push_back(card);
    }else{
      if(card.supports_hotspot && hotspot_card==std::nullopt){
        hotspot_card=card;
      }
    }
  }
  if(monitor_mode_cards.empty()){
    // try if we can find a card that at least does monitor mode (but not injection)
    for(const auto& card:discovered_cards){
      if(card.supports_monitor_mode){
        openhd::log::get_default()->warn("Using card {} for monitor mode(wifibroadcast) even though it does not support injection",card.device_name);
        monitor_mode_cards.push_back(card);
        hotspot_card=std::nullopt;
        break;
      }
    }
  }
  if(profile.is_air && monitor_mode_cards.size()>1){
    monitor_mode_cards.resize(1);
  }
  return {monitor_mode_cards,hotspot_card};
}
