#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <vector>
#include <utility>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"
#include "openhd-stream.hpp"
#include "openhd-wifi.hpp"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class is responsible for setting up all the Wifibroadcast streams needed for OpenHD.
 */
class WBStreams {
public:
    WBStreams(bool is_air, std::string unit_id);
    /**
     * Set the wifi cards for broadcasting found on the system, needs to be called before configure().
     * @param cards the broadcast wifi cards on the system.
     */
    void set_broadcast_cards(std::vector<WiFiCard> cards);
    /*
     * Call this after setting the broadcast cards to start the wifibroadcast instances.
     */
    void configure();
    void configure_telemetry();
    void configure_video();
    [[nodiscard]] std::vector<std::string> broadcast_card_names()const;
private:
    const std::string m_unit_id;
    const bool m_is_air = false;
    const int m_mcs = 3;
    std::vector<WiFiCard> m_broadcast_cards;
private:
    // For telemetry, bidirectional in opposite drections
    std::unique_ptr<UDPWBTransmitter> udpTelemetryTx;
    std::unique_ptr<UDPWBReceiver> udpTelemetryRx;
    // For video, on air there are only tx instances, on ground there are only rx instances.
    std::vector<std::unique_ptr<UDPWBTransmitter>> udpVideoTxList;
    std::vector<std::unique_ptr<UDPWBReceiver>> udpVideoRxList;
    // TODO make more configurable
    std::unique_ptr<UDPWBTransmitter> createUdpWbTx(uint8_t radio_port,int udp_port);
    [[nodiscard]] std::unique_ptr<UDPWBReceiver> createUdpWbRx(uint8_t radio_port,int udp_port) const;
};


#endif
