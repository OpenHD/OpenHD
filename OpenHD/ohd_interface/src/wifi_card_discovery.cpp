#include <iostream>
#include <regex>

#include "openhd-spdlog.hpp"
#include "openhd-util-filesystem.hpp"
#include "openhd-ignore-interfaces.h"
#include "openhd-util.hpp"
#include "wifi_card.hpp"
#include "wifi_card_discovery.h"

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

// So stephen used phy-lookup to get the supported channels of a card - however, that doesn't work reliable.
// This code uses "iwlist XXX frequency" which returns something the likes of:
// openhd@openhd:~$ iwlist wlan1 frequency
// wlan1     32 channels in total; available frequencies :
//           Channel 01 : 2.412 GHz
//                ...
// So while annoying, let's just use iw dev and parse the result
// For now, no separation into which channel(s) are supported, just weather any 2.4G or 5G frequency is supported
struct SupportedFrequency{
  bool supports_2G=false;
  bool supports_5G=false;
};
static SupportedFrequency supported_frequencies(const std::string& wifi_interface_name){
  SupportedFrequency ret{false, false};
  const std::string command="iwlist "+wifi_interface_name+" frequency";
  const auto res_op=OHDUtil::run_command_out(command.c_str());
  if(!res_op.has_value()){
    openhd::log::get_default()->warn("get_supported_channels for "+wifi_interface_name+" failed");
    return ret;
  }
  const auto& res=res_op.value();
  if(res.find("5.")!= std::string::npos){
    ret.supports_5G= true;
  }
  if(res.find("2.")!= std::string::npos){
    ret.supports_2G= true;
  }
  return ret;
}


std::vector<WiFiCard> DWifiCards::discover() {
  openhd::log::get_default()->debug("WiFi::discover()");
  std::vector<WiFiCard> m_wifi_cards;
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
        m_wifi_cards.push_back(card_opt.value());
      }
    }
  }
  /*if(true){
    WiFiCard wi_fi_card{};
    wi_fi_card.supports_2ghz=true;
    wi_fi_card.supports_5ghz= true;
    wi_fi_card.supports_injection=true;
    wi_fi_card.interface_name="wlp3s0mon";
    wi_fi_card.mac="lol";
    return {wi_fi_card};
  }*/
  openhd::log::get_default()->info("WiFi::discover done, n cards: {}",m_wifi_cards.size());
  write_wificards_manifest(m_wifi_cards);
  return m_wifi_cards;
}

std::optional<WiFiCard> DWifiCards::process_card(const std::string &interface_name) {
  std::stringstream device_uevent_file;
  device_uevent_file << "/sys/class/net/";
  device_uevent_file << interface_name.c_str();
  device_uevent_file << "/device/uevent";

  std::ifstream t(device_uevent_file.str());
  const std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());


  const std::regex driver_regex{"DRIVER=([\\w]+)"};

  std::smatch result;
  if (!std::regex_search(raw_value, result, driver_regex)) {
    openhd::log::get_default()->warn("no result");
    return std::nullopt;
  }

  if (result.size() != 2) {
    openhd::log::get_default()->warn("result doesnt match");
    return std::nullopt;
  }

  const std::string driver_name = result[1];

  WiFiCard card;
  card.interface_name = interface_name;
  card.driver_name=driver_name;

  card.type = driver_to_wifi_card_type(driver_name);

  std::stringstream phy_file;
  phy_file << "/sys/class/net/";
  phy_file << interface_name.c_str();
  phy_file << "/phy80211/index";

  std::ifstream d(phy_file.str());
  const std::string phy_val((std::istreambuf_iterator<char>(d)), std::istreambuf_iterator<char>());

  // This reported value is right in most cases, so we use it as a default. However, for example the RTL8812AU reports
  // both 2G and 5G but can only do 5G with the monitor mode driver.
  const auto supported_freq= supported_frequencies(card.interface_name);
  const bool supports_2ghz = supported_freq.supports_2G;
  const bool supports_5ghz = supported_freq.supports_5G;

  // Note that this does not neccessarily mean this info is right
  // a card might report a specific channel but then since monitor mode is so hack not support the channel in monitor mode
  openhd::log::get_default()->debug("Card {} reports driver:{} supprts_2G:{} supports_5G:{}",
                                    card.interface_name,driver_name,OHDUtil::yes_or_no(supports_2ghz),OHDUtil::yes_or_no(supports_5ghz));

  std::stringstream address;
  address << "/sys/class/net/";
  address << interface_name;
  address << "/address";

  std::ifstream f(address.str());
  std::string mac((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  OHDUtil::rtrim(mac);

  card.mac = mac;

  switch (card.type) {
    case WiFiCardType::Atheros9k: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = true;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Atheros9khtc: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = true;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Ralink: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Intel: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = false;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Broadcom: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = false;
      card.supports_hotspot = true;
      break;
    }
    case WiFiCardType::Realtek8812au: {
      // Known issue: Realtek8812au reports 2.4 and 5G, but only supports 5G in monitor mode
      // 22.10.22: Actually, rtl8812au supports both 2.4G and 5G in monitor mode, at least with our current driver branch
      card.xx_supports_5ghz=true;
      card.xx_supports_2ghz = true;
      card.supports_rts = true;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Realtek88x2bu: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    case WiFiCardType::Realtek8188eu: {
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = true;
      card.supports_hotspot = false;
      break;
    }
    default: {
      openhd::log::get_default()->warn("Unknown card type for "+card.interface_name);
      card.xx_supports_5ghz = supports_5ghz;
      card.xx_supports_2ghz = supports_2ghz;
      card.supports_rts = false;
      card.supports_injection = false;
      card.supports_hotspot = false;
      break;
    }
  }
  // rn we only support hotspot on the rpi integrated wifi adapter
  card.supports_hotspot=false;
  if(card.type==WiFiCardType::Broadcom){
    card.supports_hotspot= true;
  }
  // hacky there is something wron with phy-lookup
  if(!(card.xx_supports_2ghz  || card.xx_supports_5ghz)){
    openhd::log::get_default()->warn("Card "+card.interface_name+" reports neither 2G nor 5G, default to 2G capable");
    card.xx_supports_2ghz=true;
  }
  if(openhd::ignore::should_be_ignored_interface(card.interface_name)
      || openhd::ignore::should_be_ignored_mac(card.mac)){
    openhd::log::get_default()->info("Ignoring card {} since whitelisted by developer",card.interface_name);
    return std::nullopt;
  }
  return card;
  // A wifi card should at least one of them both.
  //if(card.supports_2ghz || card.supports_5ghz){
  //  return card;
  //}
  //return std::nullopt;
}

bool DWifiCards::any_wifi_card_supporting_monitor(const std::vector<WiFiCard>& cards) {
  for(const auto& card:cards){
    if(card.supports_injection)return true;
  }
  return false;
}

DWifiCards::ProcessedWifiCards DWifiCards::process_and_evaluate_cards(std::vector<WiFiCard> discovered_cards,bool max_one_broadcast_card) {

  std::vector<WiFiCard> monitor_mode_cards{};
  std::optional<WiFiCard> hotspot_card=std::nullopt;

  for(const auto& card:discovered_cards){
    if(card.supports_injection){
      monitor_mode_cards.push_back(card);
    }else{
      if(card.supports_hotspot && hotspot_card==std::nullopt){
        hotspot_card=card;
      }
    }
  }
  if(max_one_broadcast_card && monitor_mode_cards.size()>1){
    monitor_mode_cards.reserve(1);
  }
  return {monitor_mode_cards,hotspot_card};
}
