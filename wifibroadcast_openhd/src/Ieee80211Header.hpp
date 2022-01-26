#ifndef __WIFIBROADCAST_IEEE80211_HEADER_HPP__
#define __WIFIBROADCAST_IEEE80211_HEADER_HPP__

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <resolv.h>
#include <cstring>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <endian.h>
#include <string>
#include <vector>
#include <array>
#include <iostream>

// Wrapper around the Ieee80211 header (declared as raw array initially)
// info https://witestlab.poly.edu/blog/802-11-wireless-lan-2/
// In the way this is declared it is an IEE80211 data frame
// https://en.wikipedia.org/wiki/802.11_Frame_Types
//TODO maybe use https://elixir.bootlin.com/linux/latest/source/include/linux/ieee80211.h
class Ieee80211Header{
public:
    static constexpr auto SIZE_BYTES=24;
    //the last byte of the mac address is recycled as a port number
    static constexpr const auto SRC_MAC_LASTBYTE=15;
    static constexpr const auto DST_MAC_LASTBYTE=21;
    static constexpr const auto FRAME_SEQ_LB=22;
    static constexpr const auto FRAME_SEQ_HB=23;
    // raw data buffer
    std::array<uint8_t,SIZE_BYTES> data={
            0x08, 0x01, // first 2 bytes control fiels
            0x00, 0x00, // 2 bytes duration (has this even an effect ?!)
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // something MAC ( 6 bytes)
            0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // something MAC ( 6 bytes)
            0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // something MAC ( 6 bytes)
            0x00, 0x00,  // iee80211 sequence control ( 2 bytes )
    };
    // default constructor
    Ieee80211Header()=default;
    // write the port re-using the MAC address (which is unused for broadcast)
    // write sequence number (not used on rx right now)
    void writeParams(const uint8_t radioPort,const uint16_t seqenceNumber){
        data[SRC_MAC_LASTBYTE] = radioPort;
        data[DST_MAC_LASTBYTE] = radioPort;
        //setSequenceControl({0,seqenceNumber});
        data[FRAME_SEQ_LB] = seqenceNumber & 0xff;
        data[FRAME_SEQ_HB] = (seqenceNumber >> 8) & 0xff;
    }
    uint8_t getRadioPort()const{
        return data[SRC_MAC_LASTBYTE];
    }
    uint16_t getSequenceNumber()const{
        uint16_t ret;
        memcpy(&ret,&data[FRAME_SEQ_LB],sizeof(uint16_t));
        return ret;
    }
    const uint8_t* getData()const{
        return data.data();
    }
    constexpr std::size_t getSize()const{
        return data.size();
    }
    uint16_t getFrameControl()const{
        uint16_t ret;
        memcpy(&ret,&data[0],2);
        return ret;
    }
    uint16_t getDurationOrConnectionId()const{
        uint16_t ret;
        memcpy(&ret,&data[2],2);
        return ret;
    }
    bool isDataFrame()const{
        return data[0]==0x08 && data[1]==0x01;
    }
    //https://witestlab.poly.edu/blog/802-11-wireless-lan-2/
    //Sequence Control: Contains a 4-bit fragment number subfield, used for frag- mentation and reassembly, and a 12-bit sequence number used to number
    //frames sent between a given transmitter and receiver.
    struct SequenceControl{
        uint8_t subfield:4;
        uint16_t sequence_nr:12;
    }__attribute__ ((packed));
    static_assert(sizeof(SequenceControl)==2);
    void setSequenceControl(const SequenceControl& sequenceControl){
        memcpy(&data[FRAME_SEQ_LB],(void*)&sequenceControl,sizeof(SequenceControl));
    };
    SequenceControl getSequenceControl()const{
        SequenceControl ret;
        memcpy(&ret,&data[FRAME_SEQ_LB],sizeof(SequenceControl));
        return ret;
    }
    void printSequenceControl()const{
        const auto tmp=getSequenceControl();
        std::cout<<"SequenceControl subfield:"<<(int)tmp.subfield<<" sequenceNr:"<<(int)tmp.sequence_nr<<"\n";
    }

}__attribute__ ((packed));
static_assert(sizeof(Ieee80211Header) == Ieee80211Header::SIZE_BYTES, "ALWAYS TRUE");

static void testLol(){
    Ieee80211Header ieee80211Header;
    uint16_t seqenceNumber=0;
    for(int i=0;i<5;i++){
        ieee80211Header.data[Ieee80211Header::FRAME_SEQ_LB] = seqenceNumber & 0xff;
        ieee80211Header.data[Ieee80211Header::FRAME_SEQ_HB] = (seqenceNumber >> 8) & 0xff;
        // now print it
        ieee80211Header.printSequenceControl();
        seqenceNumber+=16;
    }
}

// Unfortunately / luckily the sequence number is overwritten by the TX. This means we can't get
// lost packets per stream, but rather lost packets per all streams only
class Ieee80211HeaderSeqNrCounter{
public:
    void onNewPacket(const Ieee80211Header& ieee80211Header){
        const auto seqCtrl=ieee80211Header.getSequenceControl();
        if(lastSeqNr==-1){
            lastSeqNr=seqCtrl.sequence_nr;
            countPacketsOutOfOrder=0;
            return;
        }
        const int32_t delta=seqCtrl.sequence_nr-lastSeqNr;
        std::cout<<"Delta: "<<delta<<"\n";
        lastSeqNr=seqCtrl.sequence_nr;
    }
private:
    int64_t lastSeqNr=-1;
    int countPacketsOutOfOrder=0;
};

namespace Ieee80211ControllFrames{
    static uint8_t u8aIeeeHeader_rts[] = {
            0xb4, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
            0xff                    // 1st byte of MAC will be overwritten with encoded port
    };
}

// hmmmm ....
// https://github.com/OpenHD/Open.HD/blob/2.0/wifibroadcast-base/tx_rawsock.c#L175
// https://github.com/OpenHD/Open.HD/blob/2.0/wifibroadcast-base/tx_telemetry.c#L144
namespace OldWifibroadcastIeee8021Header{
    static uint8_t u8aIeeeHeader_data[] = {
            0x08, 0x02, 0x00, 0x00,             // frame control field (2 bytes), duration (2 bytes)
            0xff, 0x00, 0x00, 0x00, 0x00, 0x00, // 1st byte of MAC will be overwritten with encoded port
            0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
            0x13, 0x22, 0x33, 0x44, 0x55, 0x66, // mac
            0x00, 0x00                          // IEEE802.11 seqnum, (will be overwritten later by Atheros firmware/wifi chip)
    };
    // I think this is actually this type of frame
    // https://en.wikipedia.org/wiki/Block_acknowledgement
    // has only 64*16=1024 bits payload
    static uint8_t u8aIeeeHeader_data_short[] = {
            0x08, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
            0xff                    // 1st byte of MAC will be overwritten with encoded port
    };
    // I think this is this type of frame 01 Control 1011 RTS
    // https://en.wikipedia.org/wiki/IEEE_802.11_RTS/CTS
    // however, rts frames usually don't have a payload
    static uint8_t u8aIeeeHeader_rts[] = {
            0xb4, 0x01, 0x00, 0x00, // frame control field (2 bytes), duration (2 bytes)
            0xff                    // 1st byte of MAC will be overwritten with encoded port
    };
}
#endif