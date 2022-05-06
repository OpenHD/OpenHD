#include <cstdio>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include <boost/regex.hpp>

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-ethernet.hpp"
#include "openhd-platform.hpp"
#include "openhd-log.hpp"
//#include "openhd-stream.hpp"
#include "openhd-wifi.hpp"
#include "openhd-global-constants.h"

#include "streams.h"

/*
 * This file is likely temporary, there are some unresolved questions over whether stream settings should be global
 * and therefore part of the system.conf settings file, or per-camera, in which case it might make more sense for
 * the video service to be starting the video streams.
 *
 * That's why the telemetry and video stream functions, which are somewhat duplicated, are separate. They may not end up
 * in the same place.
 *
 */
Streams::Streams(boost::asio::io_service &io_service, bool is_air, std::string unit_id):
m_io_service(io_service), m_is_air(is_air), m_unit_id(unit_id) {}


void Streams::set_broadcast_cards(std::vector<WiFiCard> cards) {
    if(!m_broadcast_cards.empty()){
        std::cerr<<"dangerous, overwriting old broadcast cards\n";
    }
    if(m_is_air && cards.size()>1){
        std::cerr<<"dangerous, the air unit should not have more than 1 wifi card for wifibroadcast\n";
    }
    if(cards.empty()){
        std::cerr<<"Without at least one wifi card, the stream(s) cannot be started\n";
    }
    m_broadcast_cards = cards;
}

void Streams::configure() {
    std::cout << "Streams::configure()" << std::endl;
    const auto broadcast_interfaces = broadcast_card_names();
    if (broadcast_interfaces.empty()) {
        ohd_log(STATUS_LEVEL_EMERGENCY, "No wifibroadcast interfaces available");
        throw std::runtime_error("no wifibroadcast interfaces available");
    }
    // Static for the moment
    configure_telemetry();
    configure_video();
}


void Streams::configure_telemetry() {
    std::cout << "Streams::configure_telemetry()" << std::endl;
    // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
    // uses 2 UDP streams in oposite directions.
    auto radioPort1=m_is_air ? OHD_TELEMETRY_WIFIBROADCAST_RF_RX_PORT_ID : OHD_TELEMETRY_WIFIBROADCAST_RF_TX_PORT_ID;
    auto radioPort2=m_is_air ? OHD_TELEMETRY_WIFIBROADCAST_RF_TX_PORT_ID : OHD_TELEMETRY_WIFIBROADCAST_RF_RX_PORT_ID;
    auto udpPort1=m_is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
    auto udpPort2=m_is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
    udpTelemetryRx= createUdpWbRx(radioPort1,udpPort1);
    udpTelemetryTx = createUdpWbTx(radioPort2,udpPort2);
    udpTelemetryRx->runInBackground();
    udpTelemetryTx->runInBackground();
}

void Streams::configure_video() {
    std::cout << "Streams::configure_video()" << std::endl;
    // Video is unidirectional, aka always goes from air pi to ground pi
    if(m_is_air){
        auto primary= createUdpWbTx(OHD_VIDEO_PRIMARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP);
        primary->runInBackground();
        auto secondary= createUdpWbTx(OHD_VIDEO_SECONDARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP);
        secondary->runInBackground();
        udpVideoTxList.push_back(std::move(primary));
        udpVideoTxList.push_back(std::move(secondary));
    }else{
        auto primary= createUdpWbRx(OHD_VIDEO_PRIMARY_RADIO_PORT, OHD_VIDEO_GROUND_VIDEO_STREAM_1_UDP);
        primary->runInBackground();
        auto secondary= createUdpWbRx(OHD_VIDEO_SECONDARY_RADIO_PORT, OHD_VIDEO_GROUND_VIDEO_STREAM_2_UDP);
        secondary->runInBackground();
        udpVideoRxList.push_back(std::move(primary));
        udpVideoRxList.push_back(std::move(secondary));
    }
}

std::vector<std::string> Streams::broadcast_card_names()const {
    std::vector<std::string> names;
    for (const auto& card : m_broadcast_cards) {
        names.push_back(card.name);
    }
    return names;
}

std::unique_ptr<UDPWBTransmitter> Streams::createUdpWbTx(uint8_t radio_port, int udp_port) {
    RadiotapHeader::UserSelectableParams wifiParams{20, false, 0, false, m_mcs};
    RadiotapHeader radiotapHeader{wifiParams};
    TOptions options{};
    options.radio_port=radio_port;
    options.keypair=std::nullopt;
    const auto cards=broadcast_card_names();
    assert(cards.size()>=1);
    options.wlan=cards.at(0);
    return std::make_unique<UDPWBTransmitter>(radiotapHeader,options,"127.0.0.1",udp_port);
}

std::unique_ptr<UDPWBReceiver> Streams::createUdpWbRx(uint8_t radio_port, int udp_port) {
    ROptions options{};
    options.radio_port=radio_port;
    options.keypair=std::nullopt;
    const auto cards=broadcast_card_names();
    assert(cards.size()>=1);
    options.rxInterfaces=cards;
    return std::make_unique<UDPWBReceiver>(options,"127.0.0.1",udp_port);
}
