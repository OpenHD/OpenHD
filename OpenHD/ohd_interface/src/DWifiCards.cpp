#include <regex>

#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

#include "DWifiCards.h"

extern "C" {
#include "nl.h"
}

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

std::vector<WiFiCard> DWifiCards::discover() {
  std::cout << "WiFi::discover()\n";
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
	  auto card= process_card(filename);
	  if(card.has_value()){
		m_wifi_cards.push_back(card.value());
	  }
	}
  }
  // Now that we have all the connected cards, we need to figure out what to use them for.
  // Fo now, just go with what we used to do in EZ-Wifibroadcast.
  for (auto &card: m_wifi_cards) {
	if (card.supports_injection) {
	  card.settings.use_for = WifiUseFor::MonitorMode;
	} else if (card.supports_hotspot) {
	  // if a card does not support injection, we use it for hotspot
	  card.settings.use_for = WifiUseFor::Hotspot;
	} else {
	  // and if a card supports neither hotspot nor injection, we use it for nothing
	  card.settings.use_for = WifiUseFor::Unknown;
	}
  }
  std::cout << "WiFi::discover done, n cards:" << m_wifi_cards.size() << "\n";
  write_wificards_manifest(m_wifi_cards);
  return m_wifi_cards;
}

std::optional<WiFiCard> DWifiCards::process_card(const std::string &interface_name) {
  std::stringstream device_file;
  device_file << "/sys/class/net/";
  device_file << interface_name.c_str();
  device_file << "/device/uevent";

  std::ifstream t(device_file.str());
  std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  std::smatch result;

  std::regex r{"DRIVER=([\\w]+)"};

  if (!std::regex_search(raw_value, result, r)) {
	std::cerr << "no result" << std::endl;
	return {};
  }

  if (result.size() != 2) {
	std::cerr << "result doesnt match" << std::endl;
	return {};
  }

  std::string driver_name = result[1];

  WiFiCard card;
  card.interface_name = interface_name;
  card.driver_name=driver_name;

  card.type = driver_to_wifi_card_type(driver_name);

  std::stringstream phy_file;
  phy_file << "/sys/class/net/";
  phy_file << interface_name.c_str();
  phy_file << "/phy80211/index";

  std::ifstream d(phy_file.str());
  std::string phy_val((std::istreambuf_iterator<char>(d)), std::istreambuf_iterator<char>());

  bool supports_2ghz = false;
  bool supports_5ghz = false;

  int ret = phy_lookup((char *)interface_name.c_str(), atoi(phy_val.c_str()), &supports_2ghz, &supports_5ghz);

  std::stringstream address;
  address << "/sys/class/net/";
  address << interface_name;
  address << "/address";

  std::ifstream f(address.str());
  std::string mac((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  boost::trim_right(mac);

  card.mac = mac;

  switch (card.type) {
    case WiFiCardType::Atheros9k: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = true;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Atheros9khtc: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = true;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Ralink: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Intel: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = false;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Broadcom: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = false;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Realtek8812au: {
	  //card.supports_5ghz = supports_5ghz;
      // For some reason, phy_lookup seems to not work when 2x RTL8812au are connected on the second card.
      card.supports_5ghz=true;
      // quirk, the driver doesn't support it for injection, we should allow it for hotspot though
	  card.supports_2ghz =false;
	  card.supports_rts = true;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Realtek88x2bu: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
        case WiFiCardType::Realtek8188eu: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = true;
	  break;
	}
	default: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = false;
	  card.supports_hotspot = true;
	  break;
	}
  }
  // rn we only support hotspot on the rpi integrated wifi adapter
  card.supports_hotspot=false;
  if(card.type==WiFiCardType::Broadcom){
	card.supports_hotspot= true;
  }
  // hacky there is something wron with phy-lookup
  if(!(card.supports_2ghz  || card.supports_5ghz)){
    card.supports_2ghz=true;
  }
  return card;
  // A wifi card should at least one of them both.
  //if(card.supports_2ghz || card.supports_5ghz){
  //  return card;
  //}
  //return std::nullopt;
}

