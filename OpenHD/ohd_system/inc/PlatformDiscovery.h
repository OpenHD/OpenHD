#ifndef PLATFORM_H
#define PLATFORM_H

#include <array>
#include <chrono>

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-wifi.hpp"
#include "openhd-ethernet.hpp"
#include "openhd-discoverable.hpp"

/**
 * Platform discovery.
 * Note: One should not use a instance of this class for anything else than discovery, to pass around the discovered
 * data use the struct from ohd_platform.
 */
class PlatformDiscovery: public OHD::IDiscoverable{
public:
    PlatformDiscovery()=default;
    virtual ~PlatformDiscovery() = default;

    void discover() override;

    nlohmann::json generate_manifest() override;

    PlatformType platform_type() {
        return m_platform_type;
    }

    BoardType board_type() {
        return m_board_type;
    }

    CarrierType carrier_type() {
        return m_carrier_type;
    }

    WiFiHotspotType wifi_hotspot_type() {
        return m_wifi_hotspot_type;
    }

    EthernetHotspotType ethernet_hotspot_type() {
        return m_ethernet_hotspot_type;
    }

private:
    void detect_raspberrypi();
    void detect_jetson();
    void detect_pc();

    PlatformType m_platform_type = PlatformTypeUnknown;
    BoardType m_board_type = BoardTypeUnknown;
    CarrierType m_carrier_type = CarrierTypeNone;

    EthernetHotspotType m_ethernet_hotspot_type = EthernetHotspotTypeNone;
    WiFiHotspotType m_wifi_hotspot_type = WiFiHotspotTypeNone;
};

#endif
