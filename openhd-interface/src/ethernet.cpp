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
#include "openhd-status.hpp"
#include "openhd-util.hpp"

#include "ethernet.h"


Ethernet::Ethernet(boost::asio::io_service &io_service): m_io_service(io_service) {}


void Ethernet::configure() {
    std::cout << "Ethernet::configure()" << std::endl;

    process_manifest();

    for (auto card : m_ethernet_cards) {
        process_card(card);
    }
}

void Ethernet::process_manifest() {
    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;

        m_is_air = j["is-air"];

    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        status_message(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        std::cerr << "Ethernet::process_manifest: " << ex.what() << std::endl;
        return;
    }

    try {
        std::ifstream f("/tmp/ethernet_manifest");
        nlohmann::json j;
        f >> j;

        m_ethernet_hotspot_type = string_to_ethernet_hotspot_type(j["hotspot"]);

        for (auto _card : j["cards"]) {

            EthernetCard card;
            card.name = _card["name"];

            m_ethernet_cards.push_back(card);

        }
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        status_message(STATUS_LEVEL_EMERGENCY, "Ethernet manifest processing failed");
            
        std::cerr << "Ethernet::process_manifest: " << ex.what() << std::endl;
        return;
    }
}


void Ethernet::process_card(EthernetCard card) {
    if (m_is_air) {
        // use for other transmission types that present as ethernet interfaces
    } else {
        // todo: handle other transmission interface types (requires new settings system support to differentiate them)
        setup_hotspot(card);
    }
}


bool Ethernet::set_card_name(EthernetCard card, std::string name) {
    std::cout << "Ethernet::set_card_name()" << std::endl;

    std::vector<std::string> args { "link", "set", card.name, "name", name };

    bool success = run_command("ip", args);

    return success;
}

/*
 * todo: signal systemd to start the hostapd and dnsmasq services afterward by signaling we've reached eth-hotspot-interface.target
 * 
 * todo: deduplicate this and similar logic in WiFi/LTE, they should be subclasses of a common base
 */
void Ethernet::setup_hotspot(EthernetCard card) {
    std::cout << "Ethernet::setup_hotspot()" << std::endl;

    if (m_hotspot_configured) {
        std::cout << "Ethernet::setup_hotspot: already configured with another card" << std::endl;
        return;
    }

    std::ostringstream message1;

    message1 << "Setting up ethernet hotspot on " << card.name << std::endl;
    status_message(STATUS_LEVEL_INFO, message1.str());

    // todo: we really shouldn't be renaming the interface, we should just inform hostapd and dnsmasq which
    //       interface to use, but this is the right way to go for now.
    bool success = set_card_name(card, "ethernethotspot0");

    if (!success) {
        status_message(STATUS_LEVEL_WARNING, "Failed to rename ethernet hotspot interface");
        std::cout << "Ethernet::setup_hotspot: renaming interface failed, hotspot disabled" << std::endl;
        return;
    }


    // todo: allow the interface address to be configured. this requires changing the dnsmasq config file though, not
    //       just the interface address. 
    std::vector<std::string> args { 
         "ethernethotspot0", m_ethernet_hotspot_address, "up",
    };

    success = run_command("ifconfig", args);

    if (!success) {
        status_message(STATUS_LEVEL_WARNING, "Failed to enable ethenet hotspot interface");
        std::cout << "Ethernet::setup_hotspot: bringing up interface failed, ethernet hotspot disabled" << std::endl;
        return;
    }

    m_hotspot_configured = true;

    status_message(STATUS_LEVEL_INFO, "Ethernet hotspot running");
}
