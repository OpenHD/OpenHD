//
// Created by consti10 on 14.04.22.
//

#include "WBEndpoint.h"

WBEndpoint::WBEndpoint(std::string TAG,const int txRadioPort,const int rxRadioPort):
MEndpoint(TAG),
txRadioPort(txRadioPort),rxRadioPort(rxRadioPort) {
#ifdef EMULATE_WIFIBROADCAST_CONNECTION
    transmitter=std::make_unique<SocketHelper::UDPForwarder>(SocketHelper::ADDRESS_LOCALHOST,txRadioPort);
    const auto cb=[this](const uint8_t* payload,const std::size_t payloadSize)mutable {
        this->parseNewData(payload,(int)payloadSize);
    };
    receiver=std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,rxRadioPort,cb);
    receiver->runInBackground();
#else
    if(txRadioPort==rxRadioPort){
        throw std::runtime_error("WBEndpoint - cannot send and receive on same radio port\n");
    }
    {
        TOptions tOptions{};
        tOptions.keypair=std::nullopt;
        tOptions.fec_k=0;
        tOptions.radio_port=txRadioPort;
        tOptions.wlan="todo";
        RadiotapHeader::UserSelectableParams wifiParams{20, false, 0, false, 1};
        RadiotapHeader radiotapHeader{wifiParams};
        wbTransmitter=std::make_unique<WBTransmitter>(radiotapHeader,tOptions);
    }
    {
        ROptions rOptions{};
        rOptions.radio_port=rxRadioPort;
        rOptions.keypair=std::nullopt;
        rOptions.rxInterfaces.push_back("todo");
        wbReceiver=std::make_unique<WBReceiver>(rOptions,[this](const uint8_t* payload,const std::size_t payloadSize){
            parseNewData(payload,payloadSize);
        });
        receiverThread=std::make_unique<std::thread>([this](){
            wbReceiver->loop();
        });
    }
#endif
    std::cout<<"WBEndpoint created tx:"<<txRadioPort<<" rx:"<<rxRadioPort<<"\n";

}

void WBEndpoint::sendMessage(const MavlinkMessage &message) {
#ifdef EMULATE_WIFIBROADCAST_CONNECTION
    if(transmitter){
        const auto data=message.pack();
        transmitter->forwardPacketViaUDP(data.data(),data.size());
    }
#else
    if(wbTransmitter){
        const auto data=message.pack();
        wbTransmitter->feedPacket(data.data(),data.size());
    }
#endif
}

std::unique_ptr<WBEndpoint> WBEndpoint::createWbEndpointOHD(const bool isAir) {
#ifdef EMULATE_WIFIBROADCAST_CONNECTION
    const std::string tag=std::string("WBEndpoint-Emu").append(isAir ? "A":"G");
    if(isAir){
        return std::make_unique<WBEndpoint>(tag,OHD_EMULATE_WB_LINK1_PORT,OHD_EMULATE_WB_LINK2_PORT);
    }else{
        return std::make_unique<WBEndpoint>(tag,OHD_EMULATE_WB_LINK2_PORT,OHD_EMULATE_WB_LINK1_PORT);
    }
#else
    const std::string tag=std::string("WBEndpoint").append(isAir ? "A":"G");
    if(isAir){
        return std::make_unique<WBEndpoint>(tag,OHD_WB_RADIO_PORT_AIR_TO_GROUND,OHD_WB_RADIO_PORT_GROUND_TO_AIR);
    }else{
        return std::make_unique<WBEndpoint>(tag,OHD_WB_RADIO_PORT_GROUND_TO_AIR,OHD_WB_RADIO_PORT_AIR_TO_GROUND);
    }
#endif
}


