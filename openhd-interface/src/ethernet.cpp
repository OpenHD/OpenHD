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


Ethernet::Ethernet(boost::asio::io_service &io_service, bool is_air, std::string unit_id): m_io_service(io_service), m_is_air(is_air), m_unit_id(unit_id) {}


void Ethernet::configure() {
    std::cout << "Ethernet::configure()" << std::endl;

    process_manifest();

    for (auto card : m_ethernet_cards) {
        process_card(card);
    }
}

void Ethernet::process_manifest() {
    try {
        std::ifstream f("/tmp/ethernet_manifest");
        nlohmann::json j;
        f >> j;

        m_ethernet_hotspot_type = string_to_ethernet_hotspot_type(j["hotspot"]);

        for (auto _card : j["cards"]) {

            EthernetCard card;
            card.name = _card["name"];
            card.mac = _card["mac"];

            m_ethernet_cards.push_back(card);

        }
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        status_message(STATUS_LEVEL_EMERGENCY, "Ethernet manifest processing failed");
            
        std::cerr << "Ethernet::process_manifest: " << ex.what() << std::endl;
        return;
    }
}


void Ethernet::process_card(EthernetCard &card) {
    if (m_is_air) {
        // use for other transmission types that present as ethernet interfaces
    } else {
        // todo: handle other transmission interface types (requires new settings system support to differentiate them)
        setup_hotspot(card);
    }
}


/* 
 * todo: deduplicate this and similar logic in WiFi/LTE, they should be subclasses of a common base, or just an inline function
 */
void Ethernet::setup_hotspot(EthernetCard &card) {
    std::cout << "Ethernet::setup_hotspot()" << std::endl;

    bool success = false;

    if (m_hotspot_configured) {
        std::cout << "Ethernet::setup_hotspot: already configured with another card" << std::endl;
        return;
    }

    std::ostringstream message1;

    message1 << "Setting up ethernet hotspot on " << card.name << std::endl;
    status_message(STATUS_LEVEL_INFO, message1.str());
    
    {
        // todo: allow the interface address to be configured. this requires changing the dnsmasq config file though, not
        //       just the interface address.
        std::vector<std::string> args { 
            card.name, m_ethernet_hotspot_address, "up",
        };

        success = run_command("ifconfig", args);

        if (!success) {
            status_message(STATUS_LEVEL_WARNING, "Failed to enable ethenet hotspot interface");
            return;
        }
    }

    {
        std::vector<std::string> args { 
            "/usr/local/share/wifibroadcast-scripts/ethernet_hotspot.sh", card.name,
        };

        success = run_command("/bin/bash", args);

        if (!success) {
            status_message(STATUS_LEVEL_WARNING, "Failed to enable dnsmasq on eth hotspot");
            return;
        }
    }

    m_hotspot_configured = true;

    status_message(STATUS_LEVEL_INFO, "Ethernet hotspot running");
}



void Ethernet::setup_static(EthernetCard &card) {
    std::cout << "Ethernet::setup_static()" << std::endl;

    bool success = false;

    std::ostringstream message1;

    message1 << "Setting up ethernet static interface " << card.name << std::endl;
    status_message(STATUS_LEVEL_INFO, message1.str());
    
    card.use_for = "static";

    if (card.ip.empty()) {
        card.ip = "192.168.3.1/24";
    }

    {
        std::vector<std::string> args { 
            card.name, card.ip, "up",
        };

        success = run_command("ifconfig", args);

        if (!success) {
            status_message(STATUS_LEVEL_WARNING, "Failed to enable ethenet interface");
            return;
        }
    }

    if (!card.gateway.empty()) {
        {
            std::vector<std::string> args { 
                "route", "add", "default", "via", card.gateway, "dev", card.name
            };

            success = run_command("ip", args);

            if (!success) {
                status_message(STATUS_LEVEL_WARNING, "Failed to enable default route on ethernet card");
                return;
            }
        }
    }

    std::ostringstream message2;

    message2 << "Ethernet static interface " << card.name << " running" << std::endl;
    status_message(STATUS_LEVEL_INFO, message2.str());
}


void Ethernet::setup_client(EthernetCard &card) {
    std::cout << "Ethernet::setup_client()" << std::endl;

    bool success = false;

    std::ostringstream message1;

    message1 << "Setting up ethernet LAN interface " << card.name << std::endl;
    status_message(STATUS_LEVEL_INFO, message1.str());
    
    card.use_for = "client";

    {
        std::vector<std::string> args { 
            "-i", card.name, "--no-ntp"
        };

        success = run_command("pump", args);

        if (!success) {
            status_message(STATUS_LEVEL_WARNING, "Failed to enable ethenet interface");
            return;
        }
    }

    std::ostringstream message2;

    message2 << "Ethernet LAN interface " << card.name << " running" << std::endl;
    status_message(STATUS_LEVEL_INFO, message2.str());
}

