
// Copyright (C) 2017, 2018, 2019 Vasily Evseenko <svpcom@p2ptech.org>
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

#include "tx.hpp"
#include "HelperSources/SchedulingHelper.hpp"
#include "HelperSources/RTPHelper.hpp"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <ctime>
#include <sys/resource.h>
#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <memory>
#include <vector>
#include <thread>

static FEC_VARIABLE_INPUT_TYPE convert(const Options& options){
    if(options.fec_k.index()==0)return FEC_VARIABLE_INPUT_TYPE::none;
    const std::string tmp=std::get<std::string>(options.fec_k);
    std::cout<<"Lol ("<<tmp<<")\n";
    if(tmp==std::string("h264")){
        return FEC_VARIABLE_INPUT_TYPE::h264;
    }else if(tmp==std::string("h265")){
        return FEC_VARIABLE_INPUT_TYPE::h265;
    }
    assert(false);
}

WBTransmitter::WBTransmitter(RadiotapHeader radiotapHeader,const Options& options1) :
        options(options1),
        mPcapTransmitter(options.wlan),
        mEncryptor(options.keypair),
        mRadiotapHeader(radiotapHeader),
        // FEC is disabled if k is integer and 0
        IS_FEC_DISABLED(options.fec_k.index() == 0 && std::get<int>(options.fec_k) == 0),
        // FEC is variable if k is an string
        IS_FEC_VARIABLE(options.fec_k.index() == 1),
        fecVariableInputType(convert(options1)){
    mEncryptor.makeNewSessionKey(sessionKeyPacket.sessionKeyNonce, sessionKeyPacket.sessionKeyData);
    if(IS_FEC_DISABLED){
        mFecDisabledEncoder=std::make_unique<FECDisabledEncoder>();
        mFecDisabledEncoder->outputDataCallback=notstd::bind_front(&WBTransmitter::sendFecPrimaryOrSecondaryFragment, this);
    }else{
        // variable if k is a string with video type
        const int kMax= options.fec_k.index() == 0 ? std::get<int>(options.fec_k) : MAX_N_P_FRAGMENTS_PER_BLOCK;
        mFecEncoder=std::make_unique<FECEncoder>(kMax,options.fec_percentage);
        mFecEncoder->outputDataCallback=notstd::bind_front(&WBTransmitter::sendFecPrimaryOrSecondaryFragment, this);
        sessionKeyPacket.MAX_N_FRAGMENTS_PER_BLOCK=FECEncoder::calculateN(kMax,options.fec_percentage);
    }
    mInputSocket= SocketHelper::openUdpSocketForReceiving(options.udp_port);
    fprintf(stderr, "WB-TX Listen on UDP Port %d assigned ID %d assigned WLAN %s\n", options.udp_port,options.radio_port,options.wlan.c_str());
    // the rx needs to know if FEC is enabled or disabled. Note, both variable and fixed fec counts as FEC enabled
    sessionKeyPacket.IS_FEC_ENABLED=!IS_FEC_DISABLED;
}

WBTransmitter::~WBTransmitter() {
    close(mInputSocket);
}


void WBTransmitter::sendPacket(const AbstractWBPacket& abstractWbPacket) {
    //std::cout << "WBTransmitter::sendPacket\n";
    mIeee80211Header.writeParams(options.radio_port, ieee80211_seq);
    ieee80211_seq += 16;
    //mIeee80211Header.printSequenceControl();

    const auto injectionTime=mPcapTransmitter.injectPacket(mRadiotapHeader,mIeee80211Header,abstractWbPacket);
    nInjectedPackets++;
#ifdef ENABLE_ADVANCED_DEBUGGING
    pcapInjectionTime.add(injectionTime);
    if(pcapInjectionTime.getMax()>std::chrono::milliseconds (1)){
        std::cerr<<"Injecting PCAP packet took really long:"<<pcapInjectionTime.getAvgReadable()<<"\n";
        pcapInjectionTime.reset();
    }
#endif
}

void WBTransmitter::sendFecPrimaryOrSecondaryFragment(const uint64_t nonce, const uint8_t* payload, const std::size_t payloadSize) {
    //std::cout << "WBTransmitter::sendFecBlock"<<(int)wbDataPacket.payloadSize<<"\n";
    const WBDataHeader wbDataHeader(nonce);
    const auto encryptedData=mEncryptor.encryptPacket(nonce,payload,payloadSize,wbDataHeader);
    //
    sendPacket({(const uint8_t*)&wbDataHeader,sizeof(WBDataHeader),encryptedData.data(),encryptedData.size()});
#ifdef ENABLE_ADVANCED_DEBUGGING
    //LatencyTestingPacket latencyTestingPacket;
    //sendPacket((uint8_t*)&latencyTestingPacket,sizeof(latencyTestingPacket));
#endif
}

void WBTransmitter::sendSessionKey() {
    std::cout << "sendSessionKey()\n";
    sendPacket({(uint8_t *)&sessionKeyPacket, WBSessionKeyPacket::SIZE_BYTES});
}

void WBTransmitter::processInputPacket(const uint8_t *buf, size_t size) {
    //std::cout << "WBTransmitter::send_packet\n";
    // this calls a callback internally
    if(IS_FEC_DISABLED){
        mFecDisabledEncoder->encodePacket(buf,size);
    }else{
        if(IS_FEC_VARIABLE){
            // variable k
            bool endBlock=false;
            if(fecVariableInputType==FEC_VARIABLE_INPUT_TYPE::h264){
                endBlock=RTPLockup::h264_end_block(buf,size);
            }else{
                endBlock=RTPLockup::h265_end_block(buf,size);
            }
            mFecEncoder->encodePacket(buf,size,endBlock);
        }else{
            // fixed k
            mFecEncoder->encodePacket(buf,size);
        }
        if(mFecEncoder->resetOnOverflow()){
            // running out of sequence numbers should never happen during the lifetime of the TX instance, but handle it properly anyways
            mEncryptor.makeNewSessionKey(sessionKeyPacket.sessionKeyNonce, sessionKeyPacket.sessionKeyData);
            sendSessionKey();
        }
    }
}

void WBTransmitter::loop() {
    constexpr auto MAX_UDP_PAYLOAD_SIZE=65507;
    // If we'd use a smaller buffer, in case the user doesn't respect the max packet size, the OS will silently drop all bytes exceeding FEC_MAX_PAYLOAD_BYTES.
    // This way we can throw an error in case the above happens.
    std::array<uint8_t,MAX_UDP_PAYLOAD_SIZE> buf{};
    std::chrono::steady_clock::time_point session_key_announce_ts{};
    std::chrono::steady_clock::time_point log_ts{};
    // send the key a couple of times on startup to increase the likeliness it is received
    bool firstTime=true;
    SocketHelper::setSocketReceiveTimeout(mInputSocket,LOG_INTERVAL);
    for(;;){
        // send the session key a couple of times on startup
        if(firstTime){
            for(int i=0;i<5;i++){
                sendSessionKey();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            firstTime=false;
        }

        // we set the timeout earlier when creating the socket
        const ssize_t message_length = recvfrom(mInputSocket, buf.data(),buf.size(), 0, nullptr, nullptr);
        if(std::chrono::steady_clock::now()>=log_ts){
            const auto runTimeMs=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-INIT_TIME).count();
            std::cout<<runTimeMs<<"\tTX "<<nPacketsFromUdpPort<<":"<<nInjectedPackets<<"\n";
            log_ts= std::chrono::steady_clock::now() + WBTransmitter::LOG_INTERVAL;
        }
        if(message_length>0){
            if(message_length>FEC_MAX_PAYLOAD_SIZE){
                throw std::runtime_error(StringFormat::convert("Error: This link doesn't support payload exceeding %d", FEC_MAX_PAYLOAD_SIZE));
            }
            nPacketsFromUdpPort++;
            const auto cur_ts=std::chrono::steady_clock::now();
            // send session key in SESSION_KEY_ANNOUNCE_DELTA intervals
            if ((cur_ts >= session_key_announce_ts) ) {
                // Announce session key
                sendSessionKey();
                session_key_announce_ts = cur_ts + SESSION_KEY_ANNOUNCE_DELTA;
            }
            processInputPacket(buf.data(), message_length);
        }else{
            if(errno==EAGAIN || errno==EWOULDBLOCK){
                // timeout
                continue;
            }
            if (errno == EINTR){
                std::cout<<"Got EINTR"<<"\n";
                continue;
            }
            throw std::runtime_error(StringFormat::convert("recvfrom error: %s", strerror(errno)));
        }
    }
}


int main(int argc, char *const *argv) {
    int opt;
    Options options{};

    RadiotapHeader::UserSelectableParams wifiParams{20, false, 0, false, 1};

    std::cout << "MAX_PAYLOAD_SIZE:" << FEC_MAX_PAYLOAD_SIZE << "\n";

    while ((opt = getopt(argc, argv, "K:k:p:u:r:B:G:S:L:M:n:")) != -1) {
        switch (opt) {
            case 'K':
                options.keypair = optarg;
                break;
            case 'k':
                if(std::string(optarg)==std::string("h264")
                || std::string(optarg)==std::string("h265")){
                    std::cout<<"LolX"<<std::string(optarg)<<"\n";
                    options.fec_k=std::string(optarg);
                }else{
                    options.fec_k=(int)std::stoi(optarg);
                }
                break;
            case 'p':
                options.fec_percentage=std::stoi(optarg);
                break;
            case 'u':
                options.udp_port = std::stoi(optarg);
                break;
            case 'r':
                options.radio_port = std::stoi(optarg);
                break;
            case 'B':
                wifiParams.bandwidth =std::stoi(optarg);
                break;
            case 'G':
                wifiParams.short_gi = (optarg[0] == 's' || optarg[0] == 'S');
                break;
            case 'S':
                wifiParams.stbc = std::stoi(optarg);
                break;
            case 'L':
                wifiParams.ldpc = std::stoi(optarg);
                break;
            case 'M':
                wifiParams.mcs_index = std::stoi(optarg);
                break;
            case 'n':
                std::cerr<<"-n is deprecated. Please read https://github.com/Consti10/wifibroadcast/blob/master/README.md \n";
                exit(1);
            default: /* '?' */
            show_usage:
                fprintf(stderr,
                        "Usage: %s [-K tx_key] [-k FEC_K] [-p FEC_PERCENTAGE] [-u udp_port] [-r radio_port] [-B bandwidth] [-G guard_interval] [-S stbc] [-L ldpc] [-M mcs_index] interface \n",
                        argv[0]);
                fprintf(stderr,
                        "Default: K='%s', k=%d, n=%d, udp_port=%d, radio_port=%d bandwidth=%d guard_interval=%s stbc=%d ldpc=%d mcs_index=%d \n",
                        "none", std::get<int>(options.fec_k), options.fec_percentage, options.udp_port, options.radio_port, wifiParams.bandwidth, wifiParams.short_gi ? "short" : "long", wifiParams.stbc, wifiParams.ldpc, wifiParams.mcs_index);
                fprintf(stderr, "Radio MTU: %lu\n", (unsigned long) FEC_MAX_PAYLOAD_SIZE);
                fprintf(stderr, "WFB version "
                WFB_VERSION
                "\n");
                exit(1);
        }
    }
    if (optind >= argc) {
        goto show_usage;
    }
    options.wlan=argv[optind];

    RadiotapHeader radiotapHeader{wifiParams};

    //RadiotapHelper::debugRadiotapHeader((uint8_t*)&radiotapHeader,sizeof(RadiotapHeader));
    //RadiotapHelper::debugRadiotapHeader((uint8_t*)&OldRadiotapHeaders::u8aRadiotapHeader80211n, sizeof(OldRadiotapHeaders::u8aRadiotapHeader80211n));
    //RadiotapHelper::debugRadiotapHeader((uint8_t*)&OldRadiotapHeaders::u8aRadiotapHeader, sizeof(OldRadiotapHeaders::u8aRadiotapHeader));
    SchedulingHelper::setThreadParamsMaxRealtime();

    if(options.fec_k.index() == 0){
        // If the user selected -k as an integer number
        const int k=std::get<int>(options.fec_k);
        if(k==0){
            std::cout<<"FEC is disabled. -p won't do anything\n";
        }else{
            const auto n=FECEncoder::calculateN(k,options.fec_percentage);
            if(n>MAX_TOTAL_FRAGMENTS_PER_BLOCK){
                std::cout<<"Please select a smaller -p (FEC_PERCENTAGE) value\n";
                exit(1);
            }
            std::cout<<"FEC is enabled and fixed. A block always consists of (K:N) fragments ("<<k<<":"<<n<<")\n";
        }
    }else{
        // If the user selected -k h264 (as a string)
        std::cout << "FEC is enabled and variable, can only be used in conjunction with h264/h265. FEC_PERCENTAGE(overhead):" << options.fec_percentage <<" type:"<<std::get<std::string>(options.fec_k)<<"\n";
        if(options.fec_percentage > 100){
            std::cout<<"Using more than 100% fec overhead (=2x the bandwidth) is not supported\n";
            //limit of the fec library, would need to go back to zfec
            exit(1);
        }
    }

    try {
        std::shared_ptr<WBTransmitter> t = std::make_shared<WBTransmitter>(
                radiotapHeader,options);
        t->loop();
    } catch (std::runtime_error &e) {
        fprintf(stderr, "Error: %s\n", e.what());
        exit(1);
    }
    return 0;
}

