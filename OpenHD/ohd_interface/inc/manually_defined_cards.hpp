//
// Created by consti10 on 13.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_MANUALLY_DEFINED_CARDS_HPP_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_MANUALLY_DEFINED_CARDS_HPP_

#include "wifi_card.hpp"
#include "openhd-spdlog.hpp"

// The helper in this namespace exist if for some reason you want to work on OpenHD and cannot rely on the autodetect of cards and what to use
// them for.

namespace openhd {

struct ManuallyDefinedWifiCards {
  // Interface name(s) of cards to use for wifibroadcast. One for air, can be multiple for ground.
  // on ground, transmission is always done on the first card (the other cards only listen passively)
  std::vector<std::string> wifibroadcast_cards;
  // Interface name of card to use for wifi hotspot, or empty if no card should be used for hotspot.
  // ! Must not contain a card already used for wifibroadcast !
  std::string hotspot_card;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ManuallyDefinedWifiCards,wifibroadcast_cards,hotspot_card);

static const std::string FILE_PATH_MANUALLY_DEFINED_CARDS=std::string(INTERFACE_SETTINGS_DIRECTORY)+std::string("manual_cards.json");

static ManuallyDefinedWifiCards get_manually_defined_cards_from_file(const std::string filename=FILE_PATH_MANUALLY_DEFINED_CARDS) {
  assert(OHDFilesystemUtil::exists(filename));
  std::ifstream f(filename);
  try {
    nlohmann::json j;
    f >> j;
    auto tmp = j.get<ManuallyDefinedWifiCards>();
    return tmp;
  } catch (nlohmann::json::exception& ex) {
    throw std::runtime_error(fmt::format("Cannot parse json. File {} is invalid",filename));
  }
  assert(false);
  return {};
}

static void write_manual_cards_template() {
  const auto file_path = "/tmp/manual_cards.json.template";
  ManuallyDefinedWifiCards example{std::vector<std::string>{"wlan0","wlan1"}, "wlan3"};
  const nlohmann::json tmp = example;
  // and write them locally for persistence
  std::ofstream t(file_path);
  t << tmp.dump(4);
  t.close();
}

static bool manually_defined_cards_file_exists(){
  return OHDFilesystemUtil::exists(FILE_PATH_MANUALLY_DEFINED_CARDS);
}

}
#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_MANUALLY_DEFINED_CARDS_HPP_
