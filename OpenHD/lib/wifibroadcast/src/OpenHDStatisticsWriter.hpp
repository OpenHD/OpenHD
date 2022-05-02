//
// Created by consti10 on 06.12.20.
//

#ifndef WIFIBROADCAST_OPENHDSTATISTICSWRITER_H
#define WIFIBROADCAST_OPENHDSTATISTICSWRITER_H

#include <cstdint>
#include "HelperSources/SocketHelper.hpp"

// TODO what happens here has to be decided yet
// write the fec decoding stats (and optionally RSSI ) for each rx stream

// Since some tx/rx instances might run in their own processes, we decided that each
// instance sends its statistics in raw form to a specific udp port (localhost)
// This data can then be picked up by the centralized mavlink service, packed into mavlink
// and forwarded as needed.
// The UDP port all instances broadcast their messages to.
static constexpr auto OHD_WIFIBROADCAST_STATISTICS_LOCAL_UDP_PORT=50000;

// Stores the min, max and average of the rssi values reported for this wifi card
// Doesn't differentiate from which antenna the rssi value came
//https://www.radiotap.org/fields/Antenna%20signal.html
class RSSIForWifiCard {
public:
    RSSIForWifiCard()=default;
    void addRSSI(int8_t rssi) {
        if (count_all == 0) {
            rssi_min = rssi;
            rssi_max = rssi;
        } else {
            rssi_min = std::min(rssi, rssi_min);
            rssi_max = std::max(rssi, rssi_max);
        }
        rssi_sum += rssi;
        count_all += 1;
    }
    int8_t getAverage()const{
        if(rssi_sum==0)return 0;
        return rssi_sum / count_all;
    }
    void reset(){
        count_all=0;
        rssi_sum=0;
        rssi_min=0;
        rssi_max=0;
    }
    int32_t count_all=0;
    int32_t rssi_sum=0;
    int8_t rssi_min=0;
    int8_t rssi_max=0;
};

class OpenHDStatisticsWriter{
private:
    SocketHelper::UDPForwarder forwarder{SocketHelper::ADDRESS_LOCALHOST, OHD_WIFIBROADCAST_STATISTICS_LOCAL_UDP_PORT};
public:
    // Forwarded data
    struct Data{
        // the unique stream ID this data refers to
        uint8_t radio_port=0;
        // all these values are absolute (like done previously in OpenHD)
        // all received packets
        uint64_t count_p_all=0;
        // n packets that were received but could not be used (after already filtering for the right port)
        uint64_t count_p_bad=0;
        // n packets that could not be decrypted
        uint64_t count_p_dec_err=0;
        // n packets that were successfully decrypted
        uint64_t count_p_dec_ok=0;
        // n packets that were corrected by FEC
        uint64_t count_p_fec_recovered=0;
        // n packets that were completely lost though FEC
        uint64_t count_p_lost=0;
        // min max and avg rssi for each wifi card since the last call.
        // if count_all for a card at position N is 0 nothing has been received on this card from the last call (or the card at position N is not used for this instance)
        std::array<RSSIForWifiCard,8> rssiPerCard{};
    }__attribute__((packed));
    void writeStats(const Data& data){
        // send all statistics regardless of the radio port to the same UDP port, they
        // will be differentiated over there
        forwarder.forwardPacketViaUDP((const uint8_t*) &data, (size_t) sizeof(data));
    }
};

#endif //WIFIBROADCAST_OPENHDSTATISTICSWRITER_H
