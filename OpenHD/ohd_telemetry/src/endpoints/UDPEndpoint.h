//
// Created by consti10 on 14.04.22.
//

#ifndef XMAVLINKSERVICE_UDPENDPOINT_H
#define XMAVLINKSERVICE_UDPENDPOINT_H

#include "MEndpoint.hpp"
#include "HelperSources/SocketHelper.hpp"
#include <thread>

// Wraps two UDP ports, one for sending and one for receiving data
// (since TCP and UART for example also allow sending and receiving).
// If this endpoint shall only send data, set the receive port to -1 and never
// call sendMessage
class UDPEndpoint : public MEndpoint{
public:
    UDPEndpoint(std::string TAG,const int senderPort,const int receiverPort);
    void sendMessage(const MavlinkMessage& message) override;
    // Makes it easy to not mess up the "what is UDP tx port on air unit is UDP rx port on ground unit" paradigm
    static std::unique_ptr<UDPEndpoint> createEndpointForOHDWifibroadcast(const bool isAir);
private:
    std::unique_ptr<SocketHelper::UDPReceiver> receiver;
    std::unique_ptr<SocketHelper::UDPForwarder> transmitter;
    const int SEND_PORT;
    const int RECV_PORT;
};


#endif //XMAVLINKSERVICE_UDPENDPOINT_H
