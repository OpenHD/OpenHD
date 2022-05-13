#include <cstdio>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <utility>

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-wifi.hpp"
#include "openhd-global-constants.h"

#include "WBStreams.h"

/*
 * This file is likely temporary, there are some unresolved questions over whether stream settings should be global
 * and therefore part of the system.conf settings file, or per-camera, in which case it might make more sense for
 * the video service to be starting the video streams.
 *
 * That's why the telemetry and video stream functions, which are somewhat duplicated, are separate. They may not end up
 * in the same place.
 *
 */
WBStreams::WBStreams(const OHDProfile& profile):
profile(profile) {}


void WBStreams::set_broadcast_card_names(const std::vector<std::string>& broadcast_cards_names) {
    m_broadcast_cards_names=broadcast_cards_names;
    if(profile.is_air && m_broadcast_cards_names.size()>1){
        std::cerr<<"dangerous, the air unit should not have more than 1 wifi card for wifibroadcast\n";
    }
    if(broadcast_cards_names.empty()){
        std::cerr<<"Without at least one wifi card, the stream(s) cannot be started\n";
    }
    if(!m_broadcast_cards_names.empty()){
        std::cerr<<"dangerous, overwriting old broadcast cards\n";
    }
    m_broadcast_cards_names=broadcast_cards_names;
}

void WBStreams::configure() {
    std::cout << "Streams::configure()" << std::endl;
    if (m_broadcast_cards_names.empty()) {
        ohd_log(STATUS_LEVEL_EMERGENCY, "WBStreams::configure:No wifibroadcast interfaces available\n");
        throw std::invalid_argument("WBStreams::configure:No wifibroadcast interfaces available");
    }
    // Static for the moment
    configure_telemetry();
    configure_video();
}


void WBStreams::configure_telemetry() {
    std::cout << "Streams::configure_telemetry()" << std::endl;
    // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
    // uses 2 UDP streams in oposite directions.
    auto radioPort1= profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
    auto radioPort2= profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
    auto udpPort1=profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
    auto udpPort2=profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
    udpTelemetryRx= createUdpWbRx(radioPort1,udpPort1);
    udpTelemetryTx = createUdpWbTx(radioPort2,udpPort2);
    udpTelemetryRx->runInBackground();
    udpTelemetryTx->runInBackground();
}

void WBStreams::configure_video() {
    std::cout << "Streams::configure_video()" << std::endl;
    // Video is unidirectional, aka always goes from air pi to ground pi
    if(profile.is_air){
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


std::unique_ptr<UDPWBTransmitter> WBStreams::createUdpWbTx(uint8_t radio_port, int udp_port) {
    RadiotapHeader::UserSelectableParams wifiParams{20, false, 0, false, m_mcs};
    RadiotapHeader radiotapHeader{wifiParams};
    TOptions options{};
    options.radio_port=radio_port;
    options.keypair=std::nullopt;
    const auto cards=m_broadcast_cards_names;
    assert(!cards.empty());
    options.wlan=cards.at(0);
    return std::make_unique<UDPWBTransmitter>(radiotapHeader,options,"127.0.0.1",udp_port);
}

std::unique_ptr<UDPWBReceiver> WBStreams::createUdpWbRx(uint8_t radio_port, int udp_port) const {
    ROptions options{};
    options.radio_port=radio_port;
    options.keypair=std::nullopt;
    const auto cards=m_broadcast_cards_names;
    assert(!cards.empty());
    options.rxInterfaces=cards;
    return std::make_unique<UDPWBReceiver>(options,"127.0.0.1",udp_port);
}
