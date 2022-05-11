//
// Created by consti10 on 13.04.22.
//

#ifndef OPENHD_TELEMETRY_AIRTELEMETRY_H
#define OPENHD_TELEMETRY_AIRTELEMETRY_H

#include "endpoints/SerialEndpoint.h"
#include "endpoints/UDPEndpoint.h"
#include "internal/InternalTelemetry.h"
#include "openhd-platform.hpp"
#include <string>

/**
 * OpenHD Air telemetry service
 */
class AirTelemetry {
public:
    explicit AirTelemetry(std::string fcSerialPort);
    // this is the main entry point for this service - it will run infinitely (until the air unit is either powered down or crashes).
    // This must NEVER crash
    void loopInfinite();
private:
    // send a mavlink message to the flight controller connected to the air unit via UART
    void sendMessageFC(MavlinkMessage& message);
    // called every time a message from the flight controller bus is received
    void onMessageFC(MavlinkMessage& message);
    // send a message to the ground pi
    void sendMessageGroundPi(MavlinkMessage& message);
    // called every time a message from the ground pi is received
    void onMessageGroundPi(MavlinkMessage& message);
private:
    static constexpr auto M_SYS_ID=OHD_SYS_ID_AIR;
    std::unique_ptr<SerialEndpoint> serialEndpoint;
    // For now, use UDP endpoint and rely on another service for starting the rx/tx links
    //std::unique_ptr<WBEndpoint> wifibroadcastEndpoint;
    std::unique_ptr<UDPEndpoint> wifibroadcastEndpoint;
    InternalTelemetry ohdTelemetryGenerator{true};
};


#endif //OPENHD_TELEMETRY_AIRTELEMETRY_H
