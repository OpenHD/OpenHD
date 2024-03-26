//
// Created by consti10 on 23.02.23.
//
#include "wifi_card.h"

#include "include_json.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(
    WiFiCardType, {
                      {WiFiCardType::UNKNOWN, nullptr},
                      {WiFiCardType::OPENHD_RTL_88X2AU, "OPENHD_RTL_88X2AU"},
                      {WiFiCardType::OPENHD_RTL_88X2BU, "OPENHD_RTL_88X2BU"},
                      {WiFiCardType::OPENHD_RTL_8852BU, "OPENHD_RTL_8852BU"},
                      {WiFiCardType::RTL_88X2AU, "RTL_88X2AU"},
                      {WiFiCardType::RTL_88X2BU, "RTL_88X2BU"},
                      {WiFiCardType::ATHEROS, "ATHEROS"},
                      {WiFiCardType::MT_7921u, "MT_7921U"},
                      {WiFiCardType::RALINK, "RALINK"},
                      {WiFiCardType::INTEL, "INTEL"},
                      {WiFiCardType::BROADCOM, "BROADCOM"},
                      {WiFiCardType::AIC, "AIC"}
                      //{WiFiCardType::, ""},
                  });

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WiFiCard, device_name, mac, phy80211_index,
                                   driver_name, type, sub_type,
                                   supported_frequencies_2G,
                                   supported_frequencies_5G)

static nlohmann::json wificards_to_json(const std::vector<WiFiCard> &cards) {
  nlohmann::json j;
  for (auto &_card : cards) {
    nlohmann::json cardJson = _card;
    j.push_back(cardJson);
  }
  return j;
}

static constexpr auto WIFI_MANIFEST_FILENAME = "/tmp/wifi_manifest";

void write_wificards_manifest(const std::vector<WiFiCard> &cards) {
  auto manifest = wificards_to_json(cards);
  OHDFilesystemUtil::write_file(WIFI_MANIFEST_FILENAME, manifest.dump(4));
}
