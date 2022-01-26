
// Copyright (C) 2017, 2018 Vasily Evseenko <svpcom@p2ptech.org>
// 2020 Constantin Geier

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 3.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "HelperSources/SchedulingHelper.hpp"
#include "FECEnabled.hpp"
#include "Encryption.hpp"
#include "HelperSources/RandomBufferPot.hpp"
#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <unistd.h>
#include <poll.h>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <list>
#include "HelperSources/Benchmark.hpp"

// Test the FEC encoding / decoding performance (throughput) of this system
// Basically measures the throughput of encoding,decoding or en&decoding FEC packets on one CPU core
// NOTE: Does not take WIFI card throughput into account


//TODO: Decode only is not implemented yet.
enum BenchmarkType{FEC_ENCODE=0,FEC_DECODE=1,ENCRYPT=2,DECRYPT=3};
static std::string benchmarkTypeReadable(const BenchmarkType value){
    switch (value) {
        case FEC_ENCODE:return "FEC_ENCODE";
        case FEC_DECODE:return "FEC_DECODE";
        //case ENCODE_AND_DECODE:return "ENCODE_AND_DECODE";
        case ENCRYPT:return "ENCRYPT";
        case DECRYPT:return "DECRYPT";
        default:return "ERROR";
    }
}

struct Options{
    // size of each packet
    int PACKET_SIZE=1446;
    int FEC_K=10;
    int FEC_PERCENTAGE=50;
    BenchmarkType benchmarkType=BenchmarkType::FEC_ENCODE;
    // How long the benchmark will take
    int benchmarkTimeSeconds=60;
};

// How many buffers we allocate (must be enough to simulate a constant flow of random data, but too many packets might result in OOM)
static std::size_t N_ALLOCATED_BUFFERS=1024;



void benchmark_fec_encode(const Options& options,bool printBlockTime=false){
    assert(options.benchmarkType==FEC_ENCODE);
    const auto testPackets=GenericHelper::createRandomDataBuffers(N_ALLOCATED_BUFFERS,options.PACKET_SIZE,options.PACKET_SIZE);
    FECEncoder encoder(options.FEC_K,options.FEC_PERCENTAGE);
    const auto cb=[](const uint64_t nonce,const uint8_t * payload,std::size_t payloadSize)mutable{
        // do nothing here. Let's hope the compiler doesn't notice.
    };
    encoder.outputDataCallback=cb;
    //
    PacketizedBenchmark packetizedBenchmark("FEC_ENCODE",(100+options.FEC_PERCENTAGE)/100.0f);
    DurationBenchmark durationBenchmark("FEC_BLOCK_ENCODE",options.PACKET_SIZE*options.FEC_K);
    const auto testBegin=std::chrono::steady_clock::now();
    packetizedBenchmark.begin();
    // run the test for X seconds
    while ((std::chrono::steady_clock::now()-testBegin)<std::chrono::seconds(options.benchmarkTimeSeconds)) {
        for (const auto &packet: testPackets) {
            durationBenchmark.start();
            bool fecPerformed=encoder.encodePacket(packet.data(), packet.size());
            if(fecPerformed){
                durationBenchmark.stop();
            }
            packetizedBenchmark.doneWithPacket(packet.size());
        }
    }
    packetizedBenchmark.end();
    durationBenchmark.print();
    //printDetail();
}


// NOTE: benchmarking the fec_decode step is not easy, since FEC is only performed if there are missing packets
// TODO do properly
void benchmark_fec_decode(const Options& options){
    // init encoder and decoder, link the callback
    FECEncoder encoder(options.FEC_K,options.FEC_PERCENTAGE);
    auto testPacketsAfterEncode=std::list<std::pair<uint64_t,std::vector<uint8_t>>>();
    const auto encoderCb=[&testPacketsAfterEncode](const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)mutable {
        testPacketsAfterEncode.push_back(std::make_pair(nonce,std::vector<uint8_t>(payload,payload+payloadSize)));
    };
    encoder.outputDataCallback=encoderCb;
    const auto testPackets=GenericHelper::createRandomDataBuffers(N_ALLOCATED_BUFFERS,options.PACKET_SIZE,options.PACKET_SIZE);
    for(const auto& packet:testPackets){
        encoder.encodePacket(packet.data(),packet.size());
    }

    FECDecoder decoder;
    const auto decoderCb=[](const uint8_t * payload,std::size_t payloadSize)mutable{
        // do nothing here. Let's hope the compiler doesn't notice.
    };
    decoder.mSendDecodedPayloadCallback=decoderCb;
    PacketizedBenchmark packetizedBenchmark("FEC_ENCODE",1.0);
    const auto testBegin=std::chrono::steady_clock::now();
    packetizedBenchmark.begin();
    while ((std::chrono::steady_clock::now()-testBegin)<std::chrono::seconds(options.benchmarkTimeSeconds)){

        for(auto& encodedPacket:testPacketsAfterEncode){
            decoder.validateAndProcessPacket(encodedPacket.first,encodedPacket.second);
            packetizedBenchmark.doneWithPacket(encodedPacket.second.size());
        }
    }
    packetizedBenchmark.end();
}

// TODO: implement decryption benchmark
void benchmark_crypt(const Options& options){
    assert(options.benchmarkType==ENCRYPT || options.benchmarkType==DECRYPT);
    Encryptor encryptor{std::nullopt};
    std::array<uint8_t,crypto_box_NONCEBYTES> sessionKeyNonce;
    std::array<uint8_t,crypto_aead_chacha20poly1305_KEYBYTES + crypto_box_MACBYTES> sessionKeyData;
    encryptor.makeNewSessionKey(sessionKeyNonce,sessionKeyData);

    constexpr auto N_BUFFERS=1000;
    RandomBufferPot randomBufferPot{N_BUFFERS,1466};
    uint64_t nonce=0;

    PacketizedBenchmark packetizedBenchmark(benchmarkTypeReadable(options.benchmarkType),1.0); // roughly 1:1
    DurationBenchmark durationBenchmark("ENC",options.PACKET_SIZE);

    const auto testBegin=std::chrono::steady_clock::now();
    packetizedBenchmark.begin();

    while ((std::chrono::steady_clock::now()-testBegin)<std::chrono::seconds(options.benchmarkTimeSeconds)){
        for(int i=0;i<N_BUFFERS;i++){
            const auto buffer=randomBufferPot.getBuffer(i);
            uint8_t add=1;
            durationBenchmark.start();
            const auto encrypted=encryptor.encryptPacket(nonce,buffer->data(),buffer->size(),add);
            durationBenchmark.stop();
            assert(encrypted.size()>0);
            nonce++;
            //
            packetizedBenchmark.doneWithPacket(buffer->size());
        }
    }
    packetizedBenchmark.end();
    durationBenchmark.print();
}

/*void benchmark_lol(const Options& options){
    static constexpr auto SIZE=1446;
    const auto N_ALLOCATED_BLOCKS=100:
    const auto N_ALLOCATED_PACKETS=options.FEC_K*100;

    const int FEC_N=options.FEC_K*options.FEC_PERCENTAGE;

    auto buffers=SemiRandomBuffers::createSemiRandomBuffers2<SIZE>(N_ALLOCATED_PACKETS);

    while (true){
        for(int i=0;i<N_ALLOCATED_BLOCKS;i++){
            st
        }
    }


}*/


int main(int argc, char *const *argv) {
    int opt;
    Options options{};

    SchedulingHelper::setThreadParamsMaxRealtime();
    SchedulingHelper::printCurrentThreadPriority("TEST_MAIN");
    SchedulingHelper::printCurrentThreadSchedulingPolicy("TEST_MAIN");

    while ((opt = getopt(argc, argv, "s:k:p:x:t:")) != -1) {
        switch (opt) {
            case 's':
                options.PACKET_SIZE = atoi(optarg);
                break;
            case 'k':
                options.FEC_K = atoi(optarg);
                break;
            case 'p':
                options.FEC_PERCENTAGE = atoi(optarg);
                break;
            case 'x':{
                options.benchmarkType=(BenchmarkType)atoi(optarg);
            }
                break;
            case 't':
                options.benchmarkTimeSeconds= atoi(optarg);
                break;
            default: /* '?' */
            show_usage:
                std::cout<<"Usage: [-s=packet size in bytes] [-k=FEC_K] [-p=FEC_P] [-x Benchmark type. 0=FEC_ENCODE 1=FEC_DECODE 2=ENCRYPT 3=DECRYPT ] [-t benchmark time in seconds]\n";
                return 1;
        }
    }

    std::cout<<"Benchmark type: "<<options.benchmarkType<<"("<<benchmarkTypeReadable(options.benchmarkType)<<")\n";
    std::cout<<"PacketSize: "<<options.PACKET_SIZE<<" B\n";
    std::cout<<"FEC_K: "<<options.FEC_K<<"\n";
    std::cout<<"FEC_PERCENTAGE: "<<options.FEC_PERCENTAGE<<"\n";
    std::cout<<"Benchmark time: "<<options.benchmarkTimeSeconds<<" s\n";
    switch (options.benchmarkType) {
        case FEC_ENCODE:
            benchmark_fec_encode(options);
            break;
        case FEC_DECODE:
            //benchmark_fec_decode(options);
            std::cout<<"Unimplemented\n";
            break;
        case ENCRYPT:
            benchmark_crypt(options);
            break;
        case DECRYPT:
            //benchmark_crypt(options);
            std::cout<<"Unimplemented\n";
            break;
    }
    return 0;
}

// Quick math:
// With a 20Mbit/s @ 60 fps one frame is on average 20*1024*1024 / 8 / 60 = 43690 bytes.
// With a max usable MTU of 1446 Bytes this means one block ideally consists of up to 443690/1446=306 packets
// if you analyze the dji link (Bitrate and resolution unknown though) you get:
// For an IDR frame: 72674 bytes, for a non-idr frame: 34648, 43647
