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
#include "openhd-wifi.hpp"
#include "openhd-profile.hpp"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class is responsible for setting up all the Wifibroadcast streams needed for OpenHD.
 * There should only be one instance of this class in the whole project.
 */
class WBStreams {
public:
    WBStreams(const OHDProfile& profile);
    /**
     * Set the names of all wifi cards for broadcasting found on the system, needs to be called before configure().
     * Note that this class expects these cards to be configured for wifibroadcast aka monitor mode with
     * injection.
     * @param broadcast_cards_names the names of all broadcast wifi cards on the system.
     */
    void set_broadcast_card_names(const std::vector<std::string>& broadcast_cards_names);
    /*
     * Call this after setting the broadcast cards to start the wifibroadcast instances.
     */
    void configure();
    void configure_telemetry();
    void configure_video();
private:
    const OHDProfile& profile;
    const int m_mcs = 3;
    std::vector<std::string> m_broadcast_cards_names;
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
