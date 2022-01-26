//
// Created by consti10 on 30.12.21.
//

// testing utility
// when run as creator, creates deterministic packets and forwards them as udp packets
// when run as validator, validates these (deterministic) packets

#include "HelperSources/RandomBufferPot.hpp"
#include "HelperSources/Helper.hpp"
#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <unistd.h>
#include <poll.h>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <thread>



// the content of each packet is simple -
// the sequence number appended by some random data depending on the sequence number

/*static bool quit = false;
static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    quit = true;
}*/

struct Options{
    // size of each packet
    int PACKET_SIZE=1446;
    // wanted bitrate (MBit/s)
    int wanted_packets_per_second=1;
    bool generator=true; // else validator
    int udp_port=5600; // port to send data to (generator) or listen on (validator)
    std::string udp_host=SocketHelper::ADDRESS_LOCALHOST;
};

using SEQUENCE_NUMBER=uint32_t;
using TIMESTAMP=uint64_t;

Options options{};


namespace TestPacket{
    // each test packet has the following layout:
    // 4 bytes (uint32_t) sequence number
    // 8 bytes (uint64_t) timestamp, nanoseconds
    // rest is semi-random data (RandomBufferPot)
    struct TestPacketHeader{
        SEQUENCE_NUMBER seqNr;
        TIMESTAMP ts;
    } __attribute__ ((packed));
    //
    static void writeTestPacketHeader(std::vector<uint8_t>& data,TestPacketHeader header){
        assert(data.size()>=sizeof(TestPacketHeader));
        // convert to network byte order
        header.seqNr= htonl(header.seqNr);
        header.ts= htole64(header.ts);
        std::memcpy(data.data(),&header.seqNr,sizeof(header.seqNr));
        //std::memcpy(data.data()+sizeof(header.seqNr),&header.ts,sizeof(header.ts));
    }
    //
    static TestPacketHeader getTestPacketHeader(const std::vector<uint8_t>& data){
        assert(data.size()>=sizeof(TestPacketHeader));
        TestPacketHeader ret;
        std::memcpy(&ret.seqNr,data.data(),sizeof(ret.seqNr));
        //std::memcpy(&ret.ts,data.data()+sizeof(ret.seqNr),sizeof(ret.ts));
        ret.seqNr= ntohl(ret.seqNr);
        //ret.ts= ntohll(ret.ts); //TODO
        return ret;
    }
    // Returns true if everything except the first couple of bytes (TestPacketHeader) match.
    // The first couple of bytes are the TestPacketHeader (which is written after creating the packet)
    bool checkPayloadMatches(const std::vector<uint8_t>& sb,const std::vector<uint8_t>& rb){
        if(sb.size()!=rb.size()){
            return false;
        }
        const int result=memcmp (&sb.data()[sizeof(TestPacketHeader)],&rb.data()[sizeof(TestPacketHeader)],sb.size()-sizeof(TestPacketHeader));
        return result==0;
    }
};

class SequenceNumberDebugger{
public:
    SequenceNumberDebugger(){
        gapsBetweenLostPackets.reserve(1000);
    }
    void sequenceNumber(const int64_t seqNr){
        nReceivedPackets++;
        auto delta=seqNr-lastReceivedSequenceNr;
        if(delta<=0){
            std::cout<<"ERROR got packet nr:"<<seqNr<<"after packet nr:"<<lastReceivedSequenceNr<<"\n";
            return;
        }
        if(delta>1){
            nLostPackets+=delta-1;
            gapsBetweenLostPackets.push_back(delta);
        }
        lastReceivedSequenceNr=seqNr;
    }
    void debug(bool clear){
        std::cout<<"N packets received:"<<nReceivedPackets<<"\tlost:"<<nLostPackets<<"\n";
        std::cout<<"Packet gaps:"<<StringHelper::vectorAsString(gapsBetweenLostPackets)<<"\n";
        if(clear){
            gapsBetweenLostPackets.resize(0);
        }
    }
private:
    std::int64_t lastReceivedSequenceNr=-1;
    std::int64_t nReceivedPackets=0;
    std::int64_t nLostPackets=0;
    std::vector<int64_t> gapsBetweenLostPackets;
};


static std::unique_ptr<RandomBufferPot> randomBufferPot= nullptr;



int main(int argc, char *const *argv) {
    int opt;

    while ((opt = getopt(argc, argv, "s:v:u:b:h:p:")) != -1) {
        switch (opt) {
            case 's':
                options.PACKET_SIZE = atoi(optarg);
                break;
            case 'v':
                options.generator = false;
                break;
            case 'u':
                options.udp_port = std::stoi(optarg);
                break;
            case 'p':
                options.wanted_packets_per_second = std::atoi(optarg);
                break;
            case 'h':
                options.udp_host=std::string(optarg);
                break;
            default: /* '?' */
            show_usage:
                std::cout<<"Usage: [-s=packet size in bytes,default:"<<options.PACKET_SIZE<<"] [-v validate packets (else generate packets)] [-u udp port,default:"<<options.udp_port<<
                "] [-h udp host default:"<<options.udp_host<<"]"<<"[-p wanted packets per second, default:"<<options.wanted_packets_per_second<<"]"<<"\n";
                return 1;
        }
    }
    if(options.PACKET_SIZE<sizeof(SEQUENCE_NUMBER)){
        std::cout<<"Error min packet size is "<<sizeof(SEQUENCE_NUMBER)<<" bytes\n";
        return 0;
    }
    const float wantedBitRate_MBits=options.PACKET_SIZE*options.wanted_packets_per_second*8.0f/1024.0f/1024.0f;
    std::cout<<"PACKET_SIZE: "<<options.PACKET_SIZE<<"\n";
    std::cout<<"wanted_packets_per_second: "<<options.wanted_packets_per_second<<"\n";
    std::cout<<"wanted Bitrate: "<<wantedBitRate_MBits<<"MBit/s"<<"\n";
    std::cout<<"Generator: "<<(options.generator ? "yes":"no")<<"\n";
    std::cout<<"UDP port: "<<options.udp_port<<"\n";
    std::cout<<"UDP host: "<<options.udp_host<<"\n";

    //RandomBufferPot randomBufferPot{10,100};
    randomBufferPot=std::make_unique<RandomBufferPot>(1000,options.PACKET_SIZE);


    const auto deltaBetweenPackets=std::chrono::nanoseconds((1000*1000*1000)/options.wanted_packets_per_second);
    auto lastLog=std::chrono::steady_clock::now();

    if(options.generator){
        static bool quit=false;
        signal(SIGTERM, [](int sig){quit=true;});
        uint32_t seqNr=0;
        SocketHelper::UDPForwarder forwarder(options.udp_host,options.udp_port);
        auto before=std::chrono::steady_clock::now();
        while (!quit){
            const auto packet= randomBufferPot->getBuffer(seqNr);
            TestPacket::writeTestPacketHeader(*packet.get(),{seqNr,0});

            forwarder.forwardPacketViaUDP(packet->data(),packet->size());
            // keep logging to a minimum for fast testing
            if(options.wanted_packets_per_second<10){
                std::cout<<"Sent packet:"<<seqNr<<"\n";
            }else{
                if(std::chrono::steady_clock::now()-lastLog>std::chrono::seconds(1)){
                    std::cout<<"Sent packets:"<<seqNr<<"\n";
                    lastLog=std::chrono::steady_clock::now();
                }
            }
            seqNr++;
            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            while (std::chrono::steady_clock::now()-before<deltaBetweenPackets){
                // busy wait
            }
            before=std::chrono::steady_clock::now();
        }
    }else{
        static int nValidPackets=0;
        static int nInvalidPackets=0;
        static auto lastLog=std::chrono::steady_clock::now();
        static SequenceNumberDebugger sequenceNumberDebugger{};

        const auto cb=[](const uint8_t* payload,const std::size_t payloadSize)mutable {

            const auto receivedPacket=std::vector<uint8_t>(payload,payload+payloadSize);

            const auto info=TestPacket::getTestPacketHeader(receivedPacket);
            sequenceNumberDebugger.sequenceNumber(info.seqNr);

            auto validPacket=randomBufferPot->getBuffer(info.seqNr);
            TestPacket::writeTestPacketHeader(*validPacket.get(),{info.seqNr,0});

            bool valid=TestPacket::checkPayloadMatches(receivedPacket,*validPacket.get());


            if(valid){
                nValidPackets++;
            }else{
                nInvalidPackets++;
                std::cout<<"Packet nr:"<< info.seqNr<<"is invalid."<<" N packets V,INV:"<<nValidPackets<<","<<nInvalidPackets<<"\n";
            }
            auto delta=std::chrono::steady_clock::now()-lastLog;
            if(delta>std::chrono::milliseconds (500)){
                //std::cout<<"Packet nr:"<< info.seqNr<<"Valid:"<<(valid ? "y":"n")<<" N packets V,INV:"<<nValidPackets<<","<<nInvalidPackets<<"\n";
                sequenceNumberDebugger.debug(true);
                lastLog=std::chrono::steady_clock::now();
            }
        };

        static SocketHelper::UDPReceiver receiver{SocketHelper::ADDRESS_LOCALHOST,options.udp_port,cb};
        signal(SIGTERM, [](int sig){receiver.stop();});
        // run until ctr+x
        receiver.start();
    }

    return 0;
}

