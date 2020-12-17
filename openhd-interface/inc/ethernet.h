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
    Ethernet(boost::asio::io_service &io_service, bool is_air, std::string unit_id);
    
    virtual ~Ethernet() {}

    void process_manifest();
    void configure();

    void process_card(EthernetCard &card);

    void setup_hotspot(EthernetCard &card);
    void setup_static(EthernetCard &card);
    void setup_client(EthernetCard &card);


private:
    boost::asio::io_service &m_io_service;

    bool m_is_air = false;

    bool m_hotspot_configured = false;

    std::string m_unit_id;

    // todo: read from settings file once new settings system merged
    std::string m_ethernet_hotspot_address = "192.168.3.1";

    std::vector<EthernetCard> m_ethernet_cards;

    EthernetHotspotType m_ethernet_hotspot_type;
};

#endif

