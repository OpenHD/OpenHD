#ifndef OPENHD_WIFI_H
#define OPENHD_WIFI_H

#include <fstream>
#include <string>

#include "include_json.hpp"
#include "openhd_platform.hpp"
#include "openhd_settings_directories.hpp"
#include "openhd_settings_persistent.hpp"
#include "openhd_util.h"
#include "openhd_util_filesystem.hpp"
#include "validate_settings_helper.h"
#include "wifi_channel.h"

// R.n (20.08) this class can be summarized as following:
// 1) WifiCard: Capabilities of a detected wifi card, no persistent settings
// 2) WifiCardSettings: What to use the wifi card for, !! no frequency settings or similar. !! (note that freq and more are figured out /stored
// by the different interface types that use wifi, e.g. WBStreams or WifiHotspot

enum class WiFiCardType {
  Unknown = 0,
  Realtek8812au,
  Realtek8814au,
  Realtek88x2bu,
  Realtek8188eu,
  Atheros9khtc,
  Atheros9k,
  Ralink,
  Intel,
  Broadcom,
};
NLOHMANN_JSON_SERIALIZE_ENUM( WiFiCardType, {
   {WiFiCardType::Unknown, nullptr},
   {WiFiCardType::Realtek8812au, "Realtek8812au"},
   {WiFiCardType::Realtek8814au, "Realtek8814au"},
   {WiFiCardType::Realtek88x2bu, "Realtek88x2bu"},
   {WiFiCardType::Realtek8188eu, "Realtek8188eu"},
   {WiFiCardType::Atheros9khtc, "Atheros9khtc"},
   {WiFiCardType::Atheros9k, "Atheros9k"},
   {WiFiCardType::Ralink, "Ralink"},
   {WiFiCardType::Intel, "Intel"},
   {WiFiCardType::Broadcom, "Broadcom"},
});

static std::string wifi_card_type_to_string(const WiFiCardType &card_type) {
  switch (card_type) {
	case WiFiCardType::Realtek8812au:return "Realtek8812au";
	case WiFiCardType::Realtek8814au:return  "Realtek8814au";
	case WiFiCardType::Realtek88x2bu:return  "Realtek88x2bu";
	case WiFiCardType::Realtek8188eu:return  "Realtek8188eu";
	case WiFiCardType::Atheros9khtc:return  "Atheros9khtc";
	case WiFiCardType::Atheros9k:return  "Atheros9k";
	case WiFiCardType::Ralink:return  "Ralink";
	case WiFiCardType::Intel:return  "Intel";
	case WiFiCardType::Broadcom:return  "Broadcom";
	default: return "unknown";
  }
}


struct WiFiCard {
  // These 3 are all (slightly different) identifiers of a card on linux.
  std::string device_name;
  std::string mac;
  // phy0, phy1,.., needed for iw commands that don't take the device name
  int phy80211_index =-1;
  // Name of the driver that runs this card.
  std::string driver_name;
  // Detected wifi card type, generated by checking known drivers.
  WiFiCardType type = WiFiCardType::Unknown;
  bool supports_monitor_mode=false;
  bool supports_injection = false;
  bool supports_hotspot = false;
  [[nodiscard]] bool supports_2GHz()const{
    return !supported_frequencies_2G.empty();
  };
  [[nodiscard]] bool supports_5GHz()const{
    return !supported_frequencies_5G.empty();
  };
  // supported 2G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_2G{};
  // supported 5G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_5G{};
  [[nodiscard]] std::vector<uint32_t> get_supported_frequencies_2G_5G()const{
    std::vector<uint32_t> ret{};
    OHDUtil::vec_append(ret,supported_frequencies_2G);
    OHDUtil::vec_append(ret,supported_frequencies_5G);
    return ret;
  };
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WiFiCard,device_name,mac,phy80211_index,driver_name,type,
                                   supports_injection,supports_monitor_mode,supports_hotspot,
                                   supported_frequencies_2G,supported_frequencies_5G)

// Only Atheros AR9271 doesn't support setting the mcs index
static bool wifi_card_supports_variable_mcs(const WiFiCard& wifi_card){
  if(wifi_card.type==WiFiCardType::Atheros9khtc || wifi_card.type==WiFiCardType::Atheros9k){
    return false;
  }
  return true;
}

// Only RTL8812au so far supports a 40Mhz channel width (and there it is also discouraged to use it)
static bool wifi_card_supports_40Mhz_channel_width(const WiFiCard& wifi_card){
  if(wifi_card.type==WiFiCardType::Realtek8812au)return true;
  return false;
}


static bool wifi_card_supports_frequency(const WiFiCard& wifi_card,const uint32_t frequency){
  const auto channel_opt=openhd::channel_from_frequency(frequency);
  if(!channel_opt.has_value()){
    openhd::log::get_default()->debug("OpenHD doesn't know frequency {}",frequency);
    return false;
  }
  const auto& channel=channel_opt.value();
  for(const auto& supported_frequency:wifi_card.get_supported_frequencies_2G_5G()){
    if(channel.frequency==supported_frequency){
      return true;
    }
  }
  openhd::log::get_default()->debug("Card {} does not support frequency {}",wifi_card.device_name,frequency);
  return false;
}


static std::string debug_cards(const std::vector<WiFiCard>& cards){
  std::stringstream ss;
  ss<<"size:"<<cards.size()<<"{";
  for(const auto& card:cards){
    ss<<card.device_name <<",";
  }
  ss<<"}";
  return ss.str();
}

static nlohmann::json wificards_to_json(const std::vector<WiFiCard> &cards) {
  nlohmann::json j;
  for (auto &_card: cards) {
	nlohmann::json cardJson = _card;
	j.push_back(cardJson);
  }
  return j;
}

static constexpr auto WIFI_MANIFEST_FILENAME = "/tmp/wifi_manifest";

static void write_wificards_manifest(const std::vector<WiFiCard> &cards) {
  auto manifest = wificards_to_json(cards);
  std::ofstream _t(WIFI_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}


#endif
