#ifndef ETHERNET_H
#define ETHERNET_H

#include <array>
#include <chrono>
#include <vector>

#include "platform.h"

#include "json.hpp"

#include "openhd-ethernet.hpp"
#include "openhd-platform.hpp"


class Ethernet {
public:
    Ethernet(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, EthernetHotspotType ethernet_hotspot_type);
    
    virtual ~Ethernet() {}

    void discover();
    void process_card(std::string interface_name);

    nlohmann::json generate_manifest();

private:
    std::vector<EthernetCard> m_ethernet_cards;

    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;
    EthernetHotspotType m_ethernet_hotspot_type;
};

#endif

