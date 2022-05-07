//
// Created by consti10 on 13.04.22.
//

#include "AirTelemetry.h"
#include "mav_helper.h"

AirTelemetry::AirTelemetry() {
    serialEndpoint=std::make_unique<SerialEndpoint>("FCSerial",SerialEndpoint::TEST_SERIAL_PORT);
    serialEndpoint->registerCallback([this](MavlinkMessage& msg){
        this->onMessageFC(msg);
    });
    // any message coming in via wifibroadcast is a message from the ground pi
    wifibroadcastEndpoint=UDPEndpoint::createEndpointForOHDWifibroadcast(true);
    wifibroadcastEndpoint->registerCallback([this](MavlinkMessage& msg){
        onMessageGroundPi(msg);
    });
    std::cout<<"Created AirTelemetry\n";
}

void AirTelemetry::sendMessageFC(MavlinkMessage& message) {
    serialEndpoint->sendMessage(message);
}

void AirTelemetry::onMessageFC(MavlinkMessage& message) {
    // forward everything to the ground pi
    sendMessageGroundPi(message);
}

void AirTelemetry::sendMessageGroundPi(MavlinkMessage& message) {
    // broadcast the mavlink message via wifibroadcast
    wifibroadcastEndpoint->sendMessage(message);
}

void AirTelemetry::onMessageGroundPi(MavlinkMessage& message) {
    const mavlink_message_t& m=message.m;
    // we do not need to forward heartbeat messages coming from the ground telemetry service,
    // They solely have a debugging purpose such that one knows the other service is alive.
    if(m.msgid==MAVLINK_MSG_ID_HEARTBEAT && m.sysid==OHD_SYS_ID_GROUND){
        // heartbeat coming from the ground service
        return;
    }
    // for now, do it as simple as possible
    sendMessageFC(message);
}

void AirTelemetry::loopInfinite() {
    while (true){
        std::cout<<"AirTelemetry::loopInfinite()\n";
        // for debugging, check if any of the endpoints is not alive
        if(wifibroadcastEndpoint){
            wifibroadcastEndpoint->debugIfAlive();
        }
        if(serialEndpoint){
            serialEndpoint->debugIfAlive();
        }
        // send heartbeat to the ground pi - everything else is handled by the callbacks and their threads
        auto heartbeat=OHDMessages::createHeartbeat(true);
        sendMessageGroundPi(heartbeat);
        auto ohdTelemetryMessages=ohdTelemetryGenerator.generateUpdates();
        for(auto& msg:ohdTelemetryMessages){
            sendMessageGroundPi(msg);
        }
        // send out in X second intervals
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

std::string AirTelemetry::uartForPlatformType(const PlatformType &platformType) {
    // we default to using a USB serial adapter on any other platform at the moment, some just need
    // to be checked to see what the port is called, but PC will likely always be USB
    std::string platformSerialPort="/dev/ttyUSB0";
    switch (platformType) {
        case PlatformTypeRaspberryPi: {
            platformSerialPort = "/dev/serial0";
            break;
        }
        case PlatformTypeJetson: {
            platformSerialPort = "/dev/ttyTHS1";
            break;
        }
        default: {
            std::cerr<<"Using default UART "<<platformSerialPort<<"\n";
            break;
        }
    }
    return platformSerialPort;
}

