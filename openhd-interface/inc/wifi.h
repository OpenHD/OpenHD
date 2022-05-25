#ifndef WIFI_H
#define WIFI_H

#include <array>
#include <chrono>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"

#include "openhd-wifi.hpp"


class WiFi {
public:
    WiFi(boost::asio::io_service &io_service, bool is_air, std::string unit_id);
    
    virtual ~WiFi() {}

    void process_manifest();
    void configure();

    void process_card(WiFiCard &card);

    void setup_hotspot(WiFiCard &card);
    void setup_client(WiFiCard &card);

    bool set_card_state(WiFiCard card, bool up);
    bool set_frequency(WiFiCard card, std::string frequency);
    bool set_txpower(WiFiCard card, std::string txpower);
    bool enable_monitor_mode(WiFiCard card);

    std::vector<WiFiCard> broadcast_cards() {
        return m_broadcast_cards;
    }

    void save_settings(std::vector<WiFiCard> cards, std::string settings_file);

private:
    boost::asio::io_service &m_io_service;

    std::string m_unit_id;

    bool m_is_air = false;

    std::vector<WiFiCard> m_wifi_cards;
    
    std::vector<WiFiCard> m_broadcast_cards;

    bool m_hotspot_configured = false;
    
    // todo: read from settings file once new settings system merged
    std::string m_wifi_hotspot_address = "192.168.2.1";
    std::string m_wifi_hotspot_txpower = "3100";

    std::string m_default_5ghz_frequency = "5180";
    std::string m_default_2ghz_frequency = "2412";

    WiFiHotspotType m_wifi_hotspot_type;
};

#endif

