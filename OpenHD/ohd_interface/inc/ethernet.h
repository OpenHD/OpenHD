#ifndef ETHERNET_H
#define ETHERNET_H

#include <array>
#include <chrono>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"

#include "openhd-ethernet.hpp"

/**
 * Provides access to the discovered ethernet cards on the system.
 * There should only be one instance of this class in the whole OpenHD project.
 */
class Ethernet {
public:
    Ethernet(bool is_air, std::string unit_id);
    virtual ~Ethernet() = default;
    void process_manifest();
    void configure();
    void setup_hotspot(EthernetCard &card);
    static void setup_static(EthernetCard &card);
    static void setup_client(EthernetCard &card);
    static void save_settings(const std::vector<EthernetCard>& cards, std::string settings_file);
private:
    void process_card(EthernetCard &card);
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

