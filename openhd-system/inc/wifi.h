#ifndef WIFI_H
#define WIFI_H

#include <array>
#include <chrono>
#include <vector>

#include "platform.h"

#include "json.hpp"

typedef enum WiFiCardType {
    WiFiCardTypeRealtek8812au,
    WiFiCardTypeRealtek8814au,
    WiFiCardTypeRealtek88x2bu,
    WiFiCardTypeRealtek8188eu,
    WiFiCardTypeAtheros9k,
    WiFiCardTypeRalink,
    WiFiCardTypeIntel,
    WiFiCardTypeUnknown
} WiFiCardType;

struct WiFiCard {
    WiFiCardType type;
    std::string name;
    bool supports_5ghz;
    bool supports_2ghz;
    bool supports_injection;
    bool supports_rts;
};


class WiFi {
public:
    WiFi(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, WiFiHotspotType wifi_hotspot_type);
    
    virtual ~WiFi() {}

    void discover();
    void process_card(std::string interface_name);

    nlohmann::json generate_manifest();

private:
    WiFiCardType string_to_wifi_card_type(std::string str);
    std::string wifi_card_type_string(WiFiCardType card_type);

    std::string wifi_hotspot_type_string(WiFiHotspotType wifi_hotspot_type);

    std::vector<WiFiCard> m_wifi_cards;

    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;
    WiFiHotspotType m_wifi_hotspot_type;
};

#endif

