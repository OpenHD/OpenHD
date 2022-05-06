#ifndef ETHERNET_H
#define ETHERNET_H

#include <array>
#include <chrono>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"

#include "openhd-ethernet.hpp"


class Ethernet {
public:
    Ethernet(bool is_air, std::string unit_id);
    
    virtual ~Ethernet() = default;

    void process_manifest();
    void configure();

    void process_card(EthernetCard &card);

    void setup_hotspot(EthernetCard &card);
    void setup_static(EthernetCard &card);
    void setup_client(EthernetCard &card);

    void save_settings(std::vector<EthernetCard> cards, std::string settings_file);

private:
    const bool m_is_air = false;
    const std::string m_unit_id;
    bool m_hotspot_configured = false;
    // todo: read from settings file once new settings system merged
    std::string m_ethernet_hotspot_address = "192.168.3.1";
    std::vector<EthernetCard> m_ethernet_cards;
    EthernetHotspotType m_ethernet_hotspot_type;
};

#endif

