//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_GROUNDTELEMETRY_H
#define OPENHD_TELEMETRY_GROUNDTELEMETRY_H

#include "endpoints/TCPEndpoint.h"
#include "endpoints/UDPEndpoint.h"
//#include "endpoints/WBEndpoint.h"
#include "endpoints/SerialEndpoint.h"
#include "ohd_telemetry//InternalTelemetry.h"

/**
 * OpenHD Ground telemetry service
 */
class GroundTelemetry {
public:
    explicit GroundTelemetry();
    // this is the main entry point for this service - it will run infinitely (until the air unit is either powered down or crashes).
    // This must NEVER crash
    void loopInfinite();
private:
    // called every time a message from the air pi is received
    void onMessageAirPi(MavlinkMessage& message);
    // send a message to the air pi
    void sendMessageAirPi(MavlinkMessage& message);
    // called every time a message is received from any of the clients connected to the Ground Station (For Example QOpenHD)
    void onMessageGroundStationClients(MavlinkMessage& message);
    // send a message to all clients connected to the ground station, for example QOpenHD
    void sendMessageGroundStationClients(MavlinkMessage& message);
private:
    static constexpr auto M_SYS_ID=OHD_SYS_ID_GROUND;
    std::unique_ptr<TCPEndpoint> tcpGroundCLient;
    std::unique_ptr<UDPEndpoint> udpGroundClient;
    // For now, use UDP endpoint and rely on another service for starting the rx/tx links
    //std::unique_ptr<WBEndpoint> wifibroadcastEndpoint;
    std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
    InternalTelemetry ohdTelemetryGenerator{true};
};


#endif //OPENHD_TELEMETRY_GROUNDTELEMETRY_H
