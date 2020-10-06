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
    Ethernet(boost::asio::io_service &io_service);
    
    virtual ~Ethernet() {}

    void process_manifest();
    void configure();

    void process_card(EthernetCard card);

    bool set_card_name(EthernetCard card, std::string name);

    void setup_hotspot(EthernetCard card);

private:
    boost::asio::io_service &m_io_service;

    bool m_is_air = false;

    // todo: read from settings file once new settings system merged
    std::string m_ethernet_hotspot_address = "192.168.3.1";

    std::vector<EthernetCard> m_ethernet_cards;
};

#endif

