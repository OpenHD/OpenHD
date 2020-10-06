#ifndef WIFI_H
#define WIFI_H

#include <array>
#include <chrono>
#include <vector>

#include "platform.h"

#include "json.hpp"

#include "openhd-types.h"
#include "openhd-structs.h"


class WiFi {
public:
    WiFi(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, WiFiHotspotType wifi_hotspot_type);
    
    virtual ~WiFi() {}

    void discover();
    void process_card(std::string interface_name);

    nlohmann::json generate_manifest();

private:
    std::vector<WiFiCard> m_wifi_cards;

    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;
    WiFiHotspotType m_wifi_hotspot_type;
};

#endif

