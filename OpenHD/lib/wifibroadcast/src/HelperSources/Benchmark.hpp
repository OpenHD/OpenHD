//
// Created by consti10 on 31.12.21.
//

#ifndef WIFIBROADCAST_PACKETIZEDBENCHMARK_H
#define WIFIBROADCAST_PACKETIZEDBENCHMARK_H

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <list>

// Helpers fo performing packet-based throughput measurement
// and/or duration-based measurement

// Wrapper for counting the packet throughput in packets per second and MBit/s.
// Print the current in 1 second intervals and the total at the end.
class PacketizedBenchmark{
public:
    /**
     * @param name1 what we are benchmarking here (for example FEC, encryption)
     * @param factor1 use a factor other than 1.0 if the packet size changes during the benchmarked step
     */
    PacketizedBenchmark(std::string name1,double factor1=1.0f):name(name1),factor(factor1){};
    void begin(){
        testBegin=std::chrono::steady_clock::now();
        logTs=std::chrono::steady_clock::now();
        int packetsDelta=0;
        int totalPacketsDelta=0;
        bytesDelta=0;
        totalBytesDelta=0;
    }
    void doneWithPacket(const double packetSizeBytes){
        packetsDelta++;
        totalPacketsDelta++;
        bytesDelta+=packetSizeBytes;
        totalBytesDelta+=packetSizeBytes;
        const auto delta=std::chrono::steady_clock::now()-logTs;
        if(delta>std::chrono::seconds(1)){
            const float currPacketsPerSecond=packetsDelta;
            const float currBitRate_MBits=bytesDelta*8.0/1024.0/1024.0;
            std::cout<<"curr. Packets per second:"<<currPacketsPerSecond<<" before "<<name<<": "<<currBitRate_MBits<<"Mbit/s";
            if(factor!=1.0f){
                std::cout<<" after "<<name<<": "<<currBitRate_MBits*factor<<"MBit/s";
            }
            std::cout<<"\n";
            logTs=std::chrono::steady_clock::now();
            packetsDelta=0;
            bytesDelta=0;
        }
    }
    void end(){
        const auto testDuration=std::chrono::steady_clock::now()-testBegin;
        const float testDurationSeconds=std::chrono::duration_cast<std::chrono::milliseconds>(testDuration).count()/1000.0f;
        //std::cout<<"Wanted duration:"<<options.benchmarkTimeSeconds<<" actual duration:"<<testDurationSeconds<<"\n";

        double totalPacketsPerSecond=totalPacketsDelta/(double)testDurationSeconds;
        double totalBitRate_MBits=totalBytesDelta*8.0/1024.0/1024.0/(double)testDurationSeconds;
        std::cout<<"Testing "<<name<<" took "<<testDurationSeconds<<" seconds"<<"\n";
        std::cout<<"TOTAL Packets per second:"<<totalPacketsPerSecond<<" before "<<name<<": "<<totalBitRate_MBits<<"Mbit/s";
        if(factor!=1.0f){
            std::cout<<" after "<<name<<": "<<totalBitRate_MBits*factor<<"MBit/s";
        }
        std::cout<<"\n";
    }
private:
    std::chrono::steady_clock::time_point testBegin=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point logTs=std::chrono::steady_clock::now();
    int packetsDelta=0;
    int totalPacketsDelta=0;
    //
    double bytesDelta=0;
    double totalBytesDelta=0;
    const std::string name;
    const double factor=1.0f;
};

// Measure how long "something" took and print the average at the end.
// E.g. encoding a FEC block, encrypting/decrypting a packet,...
class DurationBenchmark{
public:
    DurationBenchmark(std::string name1,int dataSizeBytes1):name(name1),dataSizeBytes(dataSizeBytes1){}
    void start(){
        before=std::chrono::steady_clock::now();
    }
    void stop(){
        const auto delta=std::chrono::steady_clock::now()-before;
        const auto deltaUs=std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
        //std::cout<<"Encoding a block of size:"<<StringHelper::memorySizeReadable(blockSizeBytes)<<
        //    " took "<<blockEncodingTimeUs/1000.0f<<" ms"<<"\n";
        blockEncodingTimeUsTotal+=deltaUs;
        blockEncodingTimeCount++;
    }
    void print(){
        double avgDeltaUs=(blockEncodingTimeUsTotal/blockEncodingTimeCount);
        float avgDeltaMs=avgDeltaUs/1000.0f;
        std::cout<<"Performing "<<name<<" on "<<StringHelper::memorySizeReadable(dataSizeBytes)<<
                 " took "<<avgDeltaMs<<" ms on average"<<"\n";
        //
        double emulatedThroughputMBits=1000.0/avgDeltaMs*dataSizeBytes*8/1024/1024;
        std::cout<<"This would equate to a throughput of: "<<emulatedThroughputMBits<<" Mbit/s\n";
    }
private:
    const std::string name;
    const int dataSizeBytes;
    double blockEncodingTimeUsTotal=0;
    int blockEncodingTimeCount=0;
    std::chrono::steady_clock::time_point before;
};


#endif //WIFIBROADCAST_PACKETIZEDBENCHMARK_H
