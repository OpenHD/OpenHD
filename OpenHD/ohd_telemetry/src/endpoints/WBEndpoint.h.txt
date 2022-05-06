//
// Created by consti10 on 14.04.22.
//

#ifndef XMAVLINKSERVICE_WBENDPOINT_H
#define XMAVLINKSERVICE_WBENDPOINT_H

#include "SocketHelper.hpp"
#include "MEndpoint.hpp"
#include "wb_include.h"
#include "UDPEndpoint.h"
#include <thread>
#include <memory>

// TODO: THIS ONE IS UNUSED SO FAR, RELIES ON UDPENDPOINT instead

//#define EMULATE_WIFIBROADCAST_CONNECTION

// dummy for now, this is what handles the Wifibroadcast out/in on air or ground pi.
class WBEndpoint :public MEndpoint{
public:
    /**
     * Uses 2 wifibroadcast instances (one for rx, one for tx)
     * to transmit and receive telemetry data.
     * @param txRadioPort the radio port of the tx instance
     * @param rxRadioPort the radio port of the rx instance.
     */
    explicit WBEndpoint(std::string TAG,int txRadioPort,int rxRadioPort);
    void sendMessage(const MavlinkMessage& message) override;
private:
    const int txRadioPort;
    const int rxRadioPort;
    // For debugging without a wifi card, I use UDPEndpoint as a alternative
    // to wifibroadcast.
#ifdef EMULATE_WIFIBROADCAST_CONNECTION
    std::unique_ptr<SocketHelper::UDPReceiver> receiver;
    std::unique_ptr<SocketHelper::UDPForwarder> transmitter;
    static constexpr auto OHD_EMULATE_WB_LINK1_PORT=7000;
    static constexpr auto OHD_EMULATE_WB_LINK2_PORT=7001;
#else
    std::unique_ptr<WBTransmitter> wbTransmitter;
    std::unique_ptr<WBReceiver> wbReceiver;
    std::unique_ptr<std::thread> receiverThread;
#endif
public:
public:
    // the link id for data from air to ground
    static constexpr int OHD_WB_RADIO_PORT_AIR_TO_GROUND=10;
    // same for ground to air
    static constexpr int OHD_WB_RADIO_PORT_GROUND_TO_AIR=11;
    static std::unique_ptr<WBEndpoint> createWbEndpointOHD(bool isAir);
};


#endif //XMAVLINKSERVICE_WBENDPOINT_H
