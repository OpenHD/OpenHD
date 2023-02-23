//
// Created by consti10 on 23.02.23.
//
#include "wifi_card.h"
#include "include_json.hpp"

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WiFiCard,device_name,mac,phy80211_index,driver_name,type,
								   supports_injection,supports_monitor_mode,supports_hotspot,
								   supported_frequencies_2G,supported_frequencies_5G)

static nlohmann::json wificards_to_json(const std::vector<WiFiCard> &cards) {
  nlohmann::json j;
  for (auto &_card: cards) {
	nlohmann::json cardJson = _card;
	j.push_back(cardJson);
  }
  return j;
}

static constexpr auto WIFI_MANIFEST_FILENAME = "/tmp/wifi_manifest";

void write_wificards_manifest(const std::vector<WiFiCard> &cards) {
  auto manifest = wificards_to_json(cards);
  std::ofstream _t(WIFI_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}
