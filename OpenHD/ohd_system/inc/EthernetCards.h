#ifndef ETHERNET_H
#define ETHERNET_H

#include <array>
#include <chrono>
#include <vector>

#include "Platform.h"

#include "json.hpp"

#include "openhd-ethernet.hpp"
#include "openhd-platform.hpp"

/**
 * Discovery and access to all ethernet cards on the system.
 */
class EthernetCards {
public:
    EthernetCards(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, EthernetHotspotType ethernet_hotspot_type);
    virtual ~EthernetCards() = default;
    void discover();
    void process_card(const std::string& interface_name);
    nlohmann::json generate_manifest();
private:
    // All the discovered ethernet cards
    std::vector<EthernetCard> m_ethernet_cards;
    const PlatformType m_platform_type;
    const BoardType m_board_type;
    const CarrierType m_carrier_type;
    const EthernetHotspotType m_ethernet_hotspot_type;
};

#endif

