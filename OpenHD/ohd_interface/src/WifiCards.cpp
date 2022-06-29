#include "WifiCards.h"

#include <iostream>

#include "openhd-log.hpp"
#include "openhd-settings.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"

#include "DWifiCards.h"

WifiCards::WifiCards(const OHDProfile &profile) : profile(profile) {}

void WifiCards::configure() {
  std::cout << "WifiCards::configure()" << std::endl;
  //Find out which cards are connected first
  m_wifi_cards=DWifiCards::discover();
  for(const auto& card: m_wifi_cards){
	std::stringstream message;
	message << "Detected wifi (" << wifi_card_type_to_string(card.type) << ") interface: " << card.interface_name << std::endl;
	ohd_log(STATUS_LEVEL::INFO, message.str());
  }
  // Consti10 - now do some sanity checks. No idea if and how the settings from stephen handle default values.
  for (auto &card: m_wifi_cards) {
	if (card.settings.use_for == WifiUseFor::Hotspot && profile.is_air) {
	  // There is no wifi hotspot created on the air pi
	  std::cerr << "No hotspot on air\n";
	  card.settings.use_for = WifiUseFor::Unknown;
	}
	if (card.settings.use_for == WifiUseFor::MonitorMode && !card.supports_injection) {
	  std::cerr << "Cannot use monitor mode on card that cannot inject\n";
	  card.settings.use_for = WifiUseFor::Unknown;
	}
	// Set the default frequency if not set yet.
	// If the card supports 5ghz, prefer the 5ghz band
	if (card.settings.frequency.empty()) {
	  if (card.supports_5ghz) {
		card.settings.frequency = m_default_5ghz_frequency;
	  } else {
		card.settings.frequency = m_default_2ghz_frequency;
	  }
	}
	//
	if (card.settings.txpower.empty()) {
	  // No idea where this comes from, for now use it ??!!
	  // TODO what should be the default value ?
	  card.settings.txpower = DEFAULT_WIFI_TX_POWER;
	}
  }
  // When we are done with the sanity checks, we can use the wifi card for its intended use case
  for (const auto &card: m_wifi_cards) {
	setup_card(card);
  }
}

void WifiCards::setup_card(const WiFiCard &card) {
  std::cerr << "Setup card: " << card.interface_name << std::endl;
  std::cout<<"Setup card:"<<wificard_to_json(card)<<"\n";
  switch (card.settings.use_for) {
    case WifiUseFor::MonitorMode:
	  set_card_state(card, false);
	  enable_monitor_mode(card);
	  set_card_state(card, true);
	  set_frequency(card, card.settings.frequency);
	  set_txpower(card, card.settings.txpower);
	  //m_broadcast_cards.push_back(card);
	  break;
    case WifiUseFor::Hotspot:
	  setup_hotspot(card);
	  break;
    case WifiUseFor::Unknown:
	default:
      std::cerr << "Card " << card.interface_name << " unknown use for\n";
	  return;
  }
}

void WifiCards::setup_hotspot(const WiFiCard &card) {
  std::cerr << "Setup hotspot unimplemented right now\n";
  //std::cout << "WifiCards::setup_hotspot(" << card.name << ")" << std::endl;
}

bool WifiCards::set_card_state(const WiFiCard &card, bool up) {
  std::cout << "WifiCards::set_card_state(" << up << ") for " << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"link", "set", "dev", card.interface_name, up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

bool WifiCards::set_frequency(const WiFiCard &card, const std::string &frequency) {
  std::cout << "WifiCards::set_frequency(" << frequency << ") for " << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "freq", frequency};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

bool WifiCards::set_txpower(const WiFiCard &card, const std::string &txpower) {
  std::cout << "WifiCards::set_txpower(" << txpower << ") for " << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "txpower", "fixed", txpower};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

bool WifiCards::enable_monitor_mode(const WiFiCard &card) {
  std::cout << "WifiCards::enable_monitor_mode(" << card.interface_name << ")" << std::endl;
  std::vector<std::string> args{"dev", card.interface_name, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

void WifiCards::save_settings(const std::vector<WiFiCard> &cards, const std::string &settings_file) {
  std::cerr << "Unimplemented\n";
}

std::vector<std::string> WifiCards::get_broadcast_card_names() const {
  std::vector<std::string> names;
  for (const auto &card: m_wifi_cards) {
	if (card.settings.use_for == WifiUseFor::MonitorMode) {
	  names.push_back(card.interface_name);
	}
  }
  return names;
}

std::string WifiCards::createDebug() const {
  std::stringstream ss;
  ss << "WifiCards::createDebug():N cards:" << m_wifi_cards.size() << "\n";
  return ss.str();
}
