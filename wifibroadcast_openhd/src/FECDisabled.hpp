//
// Created by consti10 on 10.03.21.
//

#ifndef WIFIBROADCAST_FECDISABLED_HPP
#define WIFIBROADCAST_FECDISABLED_HPP

#include <cstdint>
#include <cerrno>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <map>

// FEC Disabled is an optional user input (K==0). In this case, These 2 classes are used.

// usage of nonce: Simple, uint64_t number increasing with each packet
class FECDisabledEncoder{
public:
    typedef std::function<void(const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)> OUTPUT_DATA_CALLBACK;
    OUTPUT_DATA_CALLBACK outputDataCallback;
    void encodePacket(const uint8_t *buf,const size_t size) {
        outputDataCallback(currPacketIndex, buf, size);
        currPacketIndex++;
        if(currPacketIndex==std::numeric_limits<uint64_t>::max()){
            currPacketIndex=0;
        }
    }
private:
    // With a 64 bit sequence number we will NEVER overrun, no matter how long the tx/rx are running
    uint64_t currPacketIndex=0;
};

class FECDisabledDecoder{
public:
    typedef std::function<void(const uint8_t * payload,std::size_t payloadSize)> SEND_DECODED_PACKET;
    // WARNING: Don't forget to register this callback !
    SEND_DECODED_PACKET mSendDecodedPayloadCallback;
private:
    // Add a limit here to not allocate infinite amounts of memory
    static constexpr std::size_t FEC_DISABLED_MAX_SIZE_OF_MAP=100;
    std::map<uint64_t,void*> fecDisabledMapOfReceivedSeqNr;
    bool firstEverPacket=true;
public:
    //No duplicates, but packets out of order are possible
    //counting lost packets doesn't work in this mode. It should be done by the upper level
    //saves the last FEC_DISABLED_MAX_SIZE_OF_MAP sequence numbers. If the sequence number of a new packet is already inside the map, it is discarded (duplicate)
    void processRawDataBlockFecDisabled(const uint64_t packetSeq,const std::vector<uint8_t>& decrypted){
        if(firstEverPacket){
            // first ever packet. Map should be empty
            fecDisabledMapOfReceivedSeqNr.clear();
            mSendDecodedPayloadCallback(decrypted.data(), decrypted.size());
            fecDisabledMapOfReceivedSeqNr.insert({packetSeq, nullptr});
            firstEverPacket= false;
        }
        // check if packet is already known (inside the map)
        const auto search = fecDisabledMapOfReceivedSeqNr.find(packetSeq);
        if(search == fecDisabledMapOfReceivedSeqNr.end()){
            // if packet is not in the map it was not yet received(unless it is older than MAX_SIZE_OF_MAP, but that is basically impossible)
            mSendDecodedPayloadCallback(decrypted.data(), decrypted.size());
            fecDisabledMapOfReceivedSeqNr.insert({packetSeq, nullptr});
        }// else this is a duplicate
        // house keeping, do not increase size to infinity
        if(fecDisabledMapOfReceivedSeqNr.size() >= FEC_DISABLED_MAX_SIZE_OF_MAP - 1){
            // remove oldest element
            fecDisabledMapOfReceivedSeqNr.erase(fecDisabledMapOfReceivedSeqNr.begin());
        }
    }
};

#endif //WIFIBROADCAST_FECDISABLED_HPP
