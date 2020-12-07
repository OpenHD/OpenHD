 #include <cstdio>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>

#include <boost/regex.hpp>

#include "json.hpp"

#include "openhd-ethernet.hpp"
#include "openhd-platform.hpp"
#include "openhd-status.hpp"
#include "openhd-wifi.hpp"
#include "openhd-util.hpp"

#include "platform.h"
#include "wifi.h"

extern "C" {
    #include "nl.h"
}


WiFi::WiFi(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, WiFiHotspotType wifi_hotspot_type) : 
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type),
    m_wifi_hotspot_type(wifi_hotspot_type) {}


void WiFi::discover() {
    std::cout << "WiFi::discover()" << std::endl;

    /*
     * Find wifi cards, excluding specific kinds of interfaces.
     * 
     */    
    std::vector<std::string> excluded_interfaces = {
        "usb",
        "lo",
        "eth"
    };

    boost::filesystem::path net("/sys/class/net");
    for (auto &entry : boost::filesystem::directory_iterator(net)) { 
        auto interface_name = entry.path().filename().string();

        auto excluded = false;
        for (auto &excluded_interface : excluded_interfaces) {
            if (boost::algorithm::contains(interface_name, excluded_interface)) {
                excluded = true;
                break;
            }   
        }

        if (!excluded) {
            process_card(interface_name);
        }
    }
}


void WiFi::process_card(std::string interface_name) {
    std::stringstream device_file;
    device_file << "/sys/class/net/";
    device_file << interface_name.c_str();
    device_file << "/device/uevent";

    std::ifstream t(device_file.str());
    std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    boost::smatch result;

    boost::regex r { "DRIVER=([\\w]+)" };

    if (!boost::regex_search(raw_value, result, r)) {
        std::cerr << "no result" << std::endl;
        return;
    }

    if (result.size() != 2) {
        std::cerr << "result doesnt match" << std::endl;
        return;
    }

    std::string driver_name = result[1];

    WiFiCard card;
    card.name = interface_name;

    card.type = string_to_wifi_card_type(driver_name);

    std::stringstream phy_file;
    phy_file << "/sys/class/net/";
    phy_file << interface_name.c_str();
    phy_file << "/phy80211/index";

    std::ifstream d(phy_file.str());
    std::string phy_val((std::istreambuf_iterator<char>(d)), std::istreambuf_iterator<char>());    

    bool supports_2ghz = false;
    bool supports_5ghz = false;

    int ret = phy_lookup((char*)interface_name.c_str(), atoi(phy_val.c_str()), &supports_2ghz, &supports_5ghz);

    switch (card.type) {
        case WiFiCardTypeAtheros9k: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = supports_2ghz;
            card.supports_rts = true;
            card.supports_injection = true;
            card.supports_hotspot = true;
            break;
        }
        case WiFiCardTypeRalink: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = supports_2ghz;
            card.supports_rts = false;
            card.supports_injection = true;
            card.supports_hotspot = true;
            break;
        }
        case WiFiCardTypeIntel: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = supports_2ghz;
            card.supports_rts = false;
            card.supports_injection = true;
            card.supports_hotspot = false;
            break;
        }
        case WiFiCardTypeBroadcom: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = supports_2ghz;
            card.supports_rts = false;
            card.supports_injection = false;
            card.supports_hotspot = true;
            break;
        }
        case WiFiCardTypeRealtek8812au: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = false; // quirk, the driver doesn't support it for injection, we should allow it for hotspot though
            card.supports_rts = true;
            card.supports_injection = true;
            card.supports_hotspot = true;
            break;
        }
        case WiFiCardTypeRealtek88x2bu: {
            card.supports_5ghz = supports_5ghz;
            card.supports_2ghz = supports_2ghz;
            card.supports_rts = false;
            card.supports_injection = true;
            card.supports_hotspot = true;
            break;
        }
        case WiFiCardTypeRealtek8188eu: {
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
            m_wifi_hotspot_type = WiFiHotspotTypeExternal;
            break;
        }
    }  

    m_wifi_cards.push_back(card);
}


nlohmann::json WiFi::generate_manifest() {
    nlohmann::json j;

    auto wifi_cards = nlohmann::json::array();

    for (auto &_card : m_wifi_cards) {
        try {
            nlohmann::json card = { 
                {"type",               wifi_card_type_to_string(_card.type) }, 
                {"name",               _card.name },
                {"supports_5ghz",      _card.supports_5ghz },
                {"supports_2ghz",      _card.supports_2ghz },
                {"supports_injection", _card.supports_injection },
                {"supports_hotspot",   _card.supports_hotspot },
                {"supports_rts",       _card.supports_rts }
            };

            std::ostringstream message;
            message << "Detected wifi (" << wifi_card_type_to_string(_card.type) << ") interface: " << _card.name << std::endl;

            status_message(STATUS_LEVEL_INFO, message.str());

            wifi_cards.push_back(card);
        } catch (std::exception &ex) {
            std::cerr << "exception: " << ex.what() << std::endl;
        }
    }
    
    j["hotspot"] = wifi_hotspot_type_to_string(m_wifi_hotspot_type);
    j["cards"] = wifi_cards;

    return j;
}
