#include "DWifiCards.h"

#include "OHDWifiCard.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

#include <regex>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include <utility>


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
// NOTE: The ASUS CARD (for examle) reports being both 2.4 and 5G capable, but the wifibroadcast driver for sure
// is only capable of 5G. so this output is not that meaningfully.
struct SupportedFrequency{
  bool supports_2G=false;
  bool supports_5G=false;
};
static SupportedFrequency supported_frequencies(const std::string& wifi_interface_name){
  SupportedFrequency ret{false, false};
  const std::string command="iwlist "+wifi_interface_name+" frequency";
  const auto res_op=OHDUtil::run_command_out(command.c_str());
  if(!res_op.has_value()){
	std::cerr<<"get_supported_channels for "<<wifi_interface_name<<" failed\n";
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
	  auto card_opt= process_card(filename);
	  if(card_opt.has_value()){
		m_wifi_cards.push_back(card_opt.value());
	  }
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

  // This reported value is right in most cases, so we use it as a default. However, for example the RTL8812AU reports
  // both 2G and 5G but can only do 5G with the monitor mode driver.
  const auto supported_freq= supported_frequencies(card.interface_name);
  const bool supports_2ghz = supported_freq.supports_2G;
  const bool supports_5ghz = supported_freq.supports_5G;

  {
	// Note that this does not neccessarily mean this info is right, rtl8812au driver "lies" in this reagrd.
	std::stringstream ss;
	ss<<"Card "<<card.interface_name<<" reports:{"<<"supports_2G:"<<OHDUtil::yes_or_no(supports_2ghz)<<" supports_5G:"<<OHDUtil::yes_or_no(supports_2ghz)<<"}\n";
	std::cout<<ss.str();
  }

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
	  card.supports_hotspot = false;
	  break;
	}
	case WiFiCardType::Atheros9khtc: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = true;
	  card.supports_injection = true;
	  card.supports_hotspot = false;
	  break;
	}
	case WiFiCardType::Ralink: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = false;
	  break;
	}
	case WiFiCardType::Intel: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = false;
	  card.supports_hotspot = false;
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
	 // Known issue: Realtek8812au reports 2.4 and 5G, but only supports 5G in monitor mode
	  card.supports_5ghz=true;
	  card.supports_2ghz =false;
	  card.supports_rts = true;
	  card.supports_injection = true;
	  card.supports_hotspot = false;
	  break;
	}
	case WiFiCardType::Realtek88x2bu: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = false;
	  break;
	}
	case WiFiCardType::Realtek8188eu: {
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
	  card.supports_rts = false;
	  card.supports_injection = true;
	  card.supports_hotspot = false;
	  break;
	}
	default: {
	  std::cerr<<"Unknown card type for "<<card.interface_name<<"\n";
	  card.supports_5ghz = supports_5ghz;
	  card.supports_2ghz = supports_2ghz;
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
  if(!(card.supports_2ghz  || card.supports_5ghz)){
	std::cerr<<"Card "<<card.interface_name<<" reports neither 2G nor 5G, default to 2G capable \n";
	card.supports_2ghz=true;
  }
  return card;
  // A wifi card should at least one of them both.
  //if(card.supports_2ghz || card.supports_5ghz){
  //  return card;
  //}
  //return std::nullopt;
}

