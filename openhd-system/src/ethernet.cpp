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
#include "openhd-util.hpp"

#include "platform.h"
#include "ethernet.h"

extern "C" {
    #include "nl.h"
}


Ethernet::Ethernet(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, EthernetHotspotType ethernet_hotspot_type) : 
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type),
    m_ethernet_hotspot_type(ethernet_hotspot_type) {}


void Ethernet::discover() {
    std::cout << "Ethernet::discover()" << std::endl;

    /*
     * Find ethernet cards, excluding specific kinds of interfaces.
     * 
     */    
    std::vector<std::string> excluded_interfaces = {
        "wlan",
        "wifi",
        "lo"
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


void Ethernet::process_card(std::string interface_name) {
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

    EthernetCard card;
    card.name = interface_name;

    card.type = string_to_ethernet_card_type(driver_name);

    m_ethernet_cards.push_back(card);
}


nlohmann::json Ethernet::generate_manifest() {
    nlohmann::json j;

    auto ethernet_cards = nlohmann::json::array();

    for (auto &_card : m_ethernet_cards) {
        try {
            nlohmann::json card = { 
                {"type",               ethernet_card_type_to_string(_card.type) }, 
                {"name",               _card.name }
            };

            std::ostringstream message;
            message << "Detected ethernet interface: " << _card.name << std::endl;

            status_message(STATUS_LEVEL_INFO, message.str());

            ethernet_cards.push_back(card);
        } catch (std::exception &ex) {
            std::cerr << "exception: " << ex.what() << std::endl;
        }
    }
    
    j["hotspot"] = ethernet_hotspot_type_to_string(m_ethernet_hotspot_type);
    j["cards"] = ethernet_cards;

    return j;
}
