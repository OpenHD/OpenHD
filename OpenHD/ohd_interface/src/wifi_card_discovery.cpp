#include "wifi_card_discovery.h"

#include <list>
#include <regex>
#include <thread>

#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "wifi_card.h"
#include "wifi_command_helper.h"

static WiFiCardType driver_to_wifi_card_type(const std::string& driver_name) {
  // The fully supported card(s)
  if (OHDUtil::equal_after_uppercase(driver_name, "rtl88xxau_ohd")) {
    return WiFiCardType::OPENHD_RTL_88X2AU;
  }
  if (OHDUtil::equal_after_uppercase(driver_name, "rtl88x2bu_ohd")) {
    // NOTE: "rtw_8822bu" is the bad kernel driver which is fucking horrible.
    return WiFiCardType::OPENHD_RTL_88X2BU;
  }
  if (OHDUtil::equal_after_uppercase(driver_name, "rtl8852bu_ohd")) {
    // NOTE: "rtw_8822bu" is the bad kernel driver which is fucking horrible.
    return WiFiCardType::OPENHD_RTL_8852BU;
  }
  // The not supported, but perhaps working card(s)
  if (OHDUtil::contains_after_uppercase(driver_name, "ath9k")) {
    return WiFiCardType::ATHEROS;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "rt2800usb")) {
    WiFiCardType::RALINK;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "iwlwifi")) {
    return WiFiCardType::INTEL;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "brcmfmac")) {
    return WiFiCardType::BROADCOM;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "bcmsdh_sdmmc")) {
    return WiFiCardType::BROADCOM;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "aicwf_sdio")) {
    return WiFiCardType::AIC;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "88xxau")) {
    return WiFiCardType::RTL_88X2AU;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "rtw_8822bu")) {
    // NOTE: "rtw_8822bu" is the bad kernel driver which doesn't support monitor
    // mode
    return WiFiCardType::RTL_88X2BU;
  }
  if (OHDUtil::contains_after_uppercase(driver_name, "mt7921u")) {
    return WiFiCardType::MT_7921u;
  }
  return WiFiCardType::UNKNOWN;
}

static std::vector<uint32_t> supported_frequencies(const int phy_index,
                                                   bool check_2g) {
  auto channels_to_try =
      check_2g ? openhd::get_channels_2G() : openhd::get_channels_5G();
  const auto tmp = openhd::get_all_channel_frequencies(channels_to_try);
  auto supported_frequencies =
      wifi::commandhelper::iw_get_supported_frequencies(
          phy_index, openhd::get_all_channel_frequencies(channels_to_try));
  return supported_frequencies;
}

std::optional<WiFiCard> DWifiCards::fill_linux_wifi_card_identifiers(
    const std::string& interface_name) {
  // get the driver name for this card
  const auto filename_device_uevent =
      fmt::format("/sys/class/net/{}/device/uevent", interface_name);
  if (!OHDFilesystemUtil::exists(filename_device_uevent)) {
    return std::nullopt;
  }
  const auto device_uevent_content =
      OHDFilesystemUtil::read_file(filename_device_uevent);
  const std::regex driver_regex{"DRIVER=([\\w]+)"};
  std::smatch result;
  if (!std::regex_search(device_uevent_content, result, driver_regex)) {
    openhd::log::get_default()->warn("no result driver regex [{}]",
                                     device_uevent_content);
    return std::nullopt;
  }
  if (result.size() != 2) {
    openhd::log::get_default()->warn("result doesnt match");
    return std::nullopt;
  }
  const std::string driver_name = result[1];

  WiFiCard card{};
  card.device_name = interface_name;
  card.driver_name = driver_name;
  card.type = driver_to_wifi_card_type(driver_name);

  // get the phy index for this card
  const auto filename_phy_index =
      fmt::format("/sys/class/net/{}/phy80211/index", card.device_name);
  const auto phy_val = OHDFilesystemUtil::read_file(filename_phy_index);
  const auto opt_phy_phy80211_index = OHDUtil::string_to_int(phy_val);
  if (!opt_phy_phy80211_index.has_value()) {
    openhd::log::get_default()->warn("Cannot find phy index for card {}",
                                     interface_name);
    return std::nullopt;
  }
  card.phy80211_index = opt_phy_phy80211_index.value();

  // get the mac address for this card
  const auto filename_mac_address =
      fmt::format("/sys/class/net/{}/address", card.device_name);
  auto mac = OHDFilesystemUtil::read_file(filename_mac_address);
  if (mac.empty()) {
    openhd::log::get_default()->warn("Cannot find mac for card {}",
                                     interface_name);
    return std::nullopt;
  }
  OHDUtil::rtrim(mac);
  card.mac = mac;
  if (card.type == WiFiCardType::OPENHD_RTL_88X2AU) {
    const bool custom_hardware =
        OHDFilesystemUtil::exists("/boot/openhd/hardware_vtx_v20.txt") ||
        OHDPlatform::instance().is_x20();
    if (custom_hardware) {
      card.sub_type = WIFI_CARD_SUB_TYPE_RTL8812AU_X20;
    } else {
      card.sub_type = WIFI_CARD_SUB_TYPE_UNKNOWN;
    }
  }
  // Here we are done with the unique identifiers
  assert(!card.device_name.empty());
  assert(!card.mac.empty());
  assert(card.phy80211_index != -1);
  assert(!card.driver_name.empty());
  return card;
}

std::vector<WiFiCard> DWifiCards::discover_connected_wifi_cards() {
  openhd::log::get_default()->trace("WiFi::discover_connected_wifi_cards");
  std::vector<WiFiCard> wifi_cards{};
  const auto netFilenames =
      OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory("/sys/class/net");
  std::vector<std::string> valid_wifi_filenames;
  // this directory has wifi cards, ethernet, ... - filter out wifi cards
  for (const auto& filename : netFilenames) {
    if (OHDFilesystemUtil::exists(
            fmt::format("/sys/class/net/{}/phy80211", filename))) {
      valid_wifi_filenames.push_back(filename);
    }
  }
  // Try and figure out more about the card, if success, use it.
  for (const auto& filename : valid_wifi_filenames) {
    auto card_opt = process_card(filename);
    if (card_opt.has_value()) {
      wifi_cards.push_back(card_opt.value());
    }
  }
  openhd::log::get_default()->trace(
      "WiFi::discover_connected_wifi_cards done, n cards: {}",
      wifi_cards.size());
  write_wificards_manifest(wifi_cards);
  return wifi_cards;
}

std::optional<WiFiCard> DWifiCards::process_card(
    const std::string& interface_name) {
  auto card_opt = fill_linux_wifi_card_identifiers(interface_name);
  if (!card_opt.has_value()) {
    return std::nullopt;
  }
  WiFiCard card = card_opt.value();

  // This reported value is right in most cases
  /*const auto supported_freq=
  wifi::commandhelper::iw_get_supported_frequency_bands(card.device_name);
  card.xx_supports_2ghz=supported_freq.supports_any_2G;
  card.xx_supports_5ghz=supported_freq.supports_any_5G;*/
  // but we now also have a method to figure out all the supported channels
  // RTL8812AU - since the openhd driver change, it reliably supports all wifi
  // frequencies, no matter what CRDA has to say
  if (card.type == WiFiCardType::OPENHD_RTL_88X2AU ||
      card.type == WiFiCardType::OPENHD_RTL_88X2BU ||
      card.type == WiFiCardType::OPENHD_RTL_8852BU) {
    card.supported_frequencies_2G = openhd::get_all_channel_frequencies(
        openhd::get_channels_2G_legal_at_least_one_country());
    card.supported_frequencies_5G = openhd::get_all_channel_frequencies(
        openhd::get_channels_5G_legal_at_least_one_country());
  } else {
    // Ask CRDA
    card.supported_frequencies_2G =
        supported_frequencies(card.phy80211_index, true);
    card.supported_frequencies_5G =
        supported_frequencies(card.phy80211_index, false);
  }
  // Note that this does not necessarily mean this info is right/complete
  // a card might report a specific channel but then since monitor mode is so
  // hack not support the channel in monitor mode
  /*card.supports_monitor_mode =
      wifi::commandhelper::iw_supports_monitor_mode(card.phy80211_index);
  card.is_openhd_supported = is_openhd_supported(card.type);*/
  /*openhd::log::get_default()->debug("Card {} reports driver:{}
     supports_2GHz:{} supports_5GHz:{} supports_monitor_mode:{}
     openhd_supported:{}",
                                    card.device_name,card.driver_name,card.supports_2GHz(),card.supports_5GHz(),
                                    card.supports_monitor_mode,card.is_openhd_supported);*/

  // temporary,hacky, only hotspot on rpi integrated wifi
  /*if (card.type == WiFiCardType::BROADCOM || card.type == WiFiCardType::AIC) {
    card.supports_hotspot = true;
  }*/
  return card;
}

int DWifiCards::n_cards_openhd_wifibroadcast_supported(
    const std::vector<WiFiCard>& cards) {
  int ret = 0;
  for (const auto& card : cards) {
    if (card.supports_openhd_wifibroadcast()) {
      ret++;
    }
  }
  return ret;
}

// OpenHD optimization: If there are multiple RX card(s), try and show them
// always in the same order, Even if they were detected in a different order
// between boots
std::vector<WiFiCard> reorder_monitor_mode_cards(std::vector<WiFiCard> cards) {
  if (cards.size() == 1) return cards;
  // card and weather card has been consumed
  std::vector<std::pair<WiFiCard, bool>> tmp;
  for (auto& card : cards) {
    tmp.emplace_back(card, false);
  }
  std::vector<WiFiCard> ret;
  // Optimization 1: always show rtl8812au cards first
  for (auto& card : tmp) {
    if (card.first.type == WiFiCardType::OPENHD_RTL_88X2BU) {
      ret.push_back(card.first);
      card.second = true;
    }
    if (card.first.type == WiFiCardType::OPENHD_RTL_8852BU) {
      ret.push_back(card.first);
      card.second = true;
    }
  }
  // Append the rest of the card(s)
  for (auto& card : tmp) {
    if (!card.second) {
      // Card is not consumed yet
      ret.push_back(card.first);
    }
  }
  return ret;
}

DWifiCards::ProcessedWifiCards DWifiCards::process_and_evaluate_cards(
    const std::vector<WiFiCard>& discovered_cards, const OHDProfile& profile) {
  // We need to figure out what's the best usage for the card(s) connected to
  // the system based on their capabilities.
  std::vector<WiFiCard> monitor_mode_cards{};
  std::optional<WiFiCard> hotspot_card = std::nullopt;
  const int n_openhd_wifibroadcast_supported_cards =
      n_cards_openhd_wifibroadcast_supported(discovered_cards);
  if (n_openhd_wifibroadcast_supported_cards <= 0) {
    // no monitor mode cards - use any other card(s) for hotspot
    if (!discovered_cards.empty()) {
      hotspot_card = discovered_cards.at(0);
    }
  } else {
    for (auto& card : discovered_cards) {
      if (card.supports_openhd_wifibroadcast()) {
        monitor_mode_cards.push_back(card);
      } else {
        if (hotspot_card == std::nullopt) {
          hotspot_card = card;
        }
      }
    }
  }
  if (profile.is_air && monitor_mode_cards.size() > 1) {
    monitor_mode_cards.resize(1);
  }
  return {reorder_monitor_mode_cards(monitor_mode_cards), hotspot_card};
}

static WiFiCard wait_for_card(const std::string& interface_name) {
  while (true) {
    auto card = DWifiCards::process_card(interface_name);
    if (card) {
      return card.value();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    openhd::log::get_default()->debug("Waiting for {}", interface_name);
  }
}

DWifiCards::ProcessedWifiCards DWifiCards::find_cards_from_manual_file(
    const std::vector<std::string>& wifibroadcast_cards,
    const std::string& opt_hotspot_card) {
  ProcessedWifiCards ret{};
  for (const auto& interface : wifibroadcast_cards) {
    auto card = wait_for_card(interface);
    ret.monitor_mode_cards.push_back(card);
  };
  if (!opt_hotspot_card.empty()) {
    auto card = wait_for_card(opt_hotspot_card);
    ret.hotspot_card = card;
  }
  return ret;
}

WiFiCard DWifiCards::create_card_monitor_emulate() {
  WiFiCard ret{};
  ret.type = WiFiCardType::OPENHD_EMULATED;
  ret.driver_name = "dummy";
  ret.supported_frequencies_2G = openhd::get_all_channel_frequencies(
      openhd::get_channels_2G_legal_at_least_one_country());
  ret.supported_frequencies_5G = openhd::get_all_channel_frequencies(
      openhd::get_channels_5G_legal_at_least_one_country());
  return ret;
}

void DWifiCards::main_discover_an_process_wifi_cards(
    const openhd::Config& config, const OHDProfile& m_profile,
    std::shared_ptr<spdlog::logger>& m_console,
    std::vector<WiFiCard>& m_monitor_mode_cards,
    std::optional<WiFiCard>& m_opt_hotspot_card) {
  const auto m_platform = OHDPlatform::instance();
  m_console->debug("Waiting for wifi card(s)...");
  const bool debug = false;
  if (config.WIFI_MONITOR_CARD_EMULATE) {
    m_monitor_mode_cards.push_back(DWifiCards::create_card_monitor_emulate());
    m_opt_hotspot_card = std::nullopt;
    if (config.WIFI_FORCE_NO_LINK_BUT_HOTSPOT) {
      auto connected_cards = DWifiCards::discover_connected_wifi_cards();
      for (auto& connected_card : connected_cards) {
        if (m_opt_hotspot_card == std::nullopt) {
          m_opt_hotspot_card = connected_card;
        }
      }
    }
    return;
  }
  if (!config.WIFI_ENABLE_AUTODETECT) {
    // Much easier to do, no weird trying to figure out what to use the card(s)
    // for Much easier to do, no weird trying to figure out what to use the
    // card(s) for
    auto processed = DWifiCards::find_cards_from_manual_file(
        config.WIFI_WB_LINK_CARDS, config.WIFI_WIFI_HOTSPOT_CARD);
    m_monitor_mode_cards = processed.monitor_mode_cards;
    m_opt_hotspot_card = processed.hotspot_card;
    if (m_profile.is_air && m_monitor_mode_cards.size() > 1) {
      m_console->warn("WB only supports one wifi card on air");
      m_monitor_mode_cards.resize(1);
    }
    return;
  }
  // We need to discover the connected cards and reason about their usage
  // Find out which cards are connected first
  auto connected_cards = DWifiCards::discover_connected_wifi_cards();
  // Issue on rpi with Atheros: For some reason, openhd is sometimes started
  // before the card finishes some initialization steps ?! and is therefore not
  // discovered. Change January 05, 23: We always wait for a card doing monitor
  // mode unless a (developer) has specified the option to do otherwise (which
  // can be usefully for testing, but is not a behaviour we want when running on
  // a user image)
  const auto begin = std::chrono::steady_clock::now();
  while (true) {
    const auto n_openhd_supported_cards =
        DWifiCards::n_cards_openhd_wifibroadcast_supported(connected_cards);
    // On the air unit, we stop the discovery as soon as we have one wb capable
    // card
    if (m_profile.is_air && n_openhd_supported_cards >= 1) {
      break;
    }
    // On the ground unit, we stop the discovery as soon as we have 2 or more wb
    // capable card(s), or timeout
    if (m_profile.is_ground() && n_openhd_supported_cards >= 2) {
      break;
    }
    const auto elapsed = std::chrono::steady_clock::now() - begin;
    if (debug) {
      const auto message = fmt::format("Waiting for supported WiFi, Found:{}",
                                       n_openhd_supported_cards);
      if (elapsed > std::chrono::seconds(3)) {
        m_console->warn(message);
      } else {
        m_console->debug(message);
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connected_cards = DWifiCards::discover_connected_wifi_cards();
    // after 10 seconds, we stop - if we didn't find a openhd wifibroadcast
    // supported card, we are not functional
    if (elapsed > std::chrono::seconds(10)) {
      if (DWifiCards::n_cards_openhd_wifibroadcast_supported(connected_cards) <=
          0) {
        m_console->warn("No openhd wifibroadcast card found");
        m_console->warn("Link not functional");
      }
      break;
    }
  }
  // now decide what to use the card(s) for
  const auto evaluated =
      DWifiCards::process_and_evaluate_cards(connected_cards, m_profile);
  m_monitor_mode_cards = evaluated.monitor_mode_cards;
  m_opt_hotspot_card = evaluated.hotspot_card;
  if (m_monitor_mode_cards.empty()) {
    m_monitor_mode_cards.push_back(DWifiCards::create_card_monitor_emulate());
  }
}
