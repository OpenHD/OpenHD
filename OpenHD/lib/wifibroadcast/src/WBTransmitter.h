#ifndef CONSTI10_WIFIBROADCAST_WB_TRANSMITTER_H
#define CONSTI10_WIFIBROADCAST_WB_TRANSMITTER_H
//
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

#include "Encryption.hpp"
#include "FECEnabled.hpp"
#include "FECDisabled.hpp"
#include "HelperSources/Helper.hpp"
#include "RawTransmitter.hpp"
#include "HelperSources/TimeHelper.hpp"
#include "wifibroadcast.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cerrno>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <variant>
#include <thread>

// Note: The UDP port is missing as an option here, since it is not an option for WFBTransmitter anymore.
// Only an option when you run this program via the command line.
struct TOptions{
    // the radio port is what is used as an index to multiplex multiple streams (telemetry,video,...)
    // into the one wfb stream
    uint8_t radio_port = 1;
    // file for encryptor
    // make optional for ease of use - with no keypair given the default "seed" is used
    std::optional<std::string> keypair=std::nullopt;
    // wlan interface to send packets with
    std::string wlan;
    // either fixed or variable. If int==fixed, if string==variable but hook needs to be added (currently only hooked h264 and h265)
    std::variant<int,std::string> fec_k=8;
    int fec_percentage=50;
};
enum FEC_VARIABLE_INPUT_TYPE{none,h264,h265};

class WBTransmitter {
public:
    /**
     * Each instance has to be assigned with a Unique ID to differentiate between streams on the RX
     * It does all the FEC encoding & encryption for this stream, then uses PcapTransmitter to inject the generated packets
     * FEC can be either enabled or disabled.
     * When run as an executable from the command line, a UDPReceiver is created for forwarding data to an instance of this class.
     * @param radiotapHeader the radiotap header that is used for injecting, contains configurable data like the mcs index.
     * @param options1 options for this instance, some of them are forwarded to the receiver instance.
     */
    WBTransmitter(RadiotapHeader radiotapHeader,const TOptions& options1);
    ~WBTransmitter();
    /**
     * feed a new packet to this instance.
     * Depending on the selected mode, this might add FEC packets or similar.
     * If the packet size exceeds the max packet size, the packet is dropped.
     * @param buf packet data buffer
     * @param size packet data buffer size
     */
    void feedPacket(const uint8_t *buf, size_t size);
private:
    const TOptions& options;
    // send the current session key via WIFI (located in mEncryptor)
    void sendSessionKey();
    // for the FEC encoder
    void sendFecPrimaryOrSecondaryFragment(const uint64_t nonce, const uint8_t* payload,const size_t payloadSize);
    // send packet by prefixing data with the current IEE and Radiotap header
    void sendPacket(const AbstractWBPacket& abstractWbPacket);
    // print some simple debug information. Called in regular intervals by the logAliveThread
    void logAlive();
    // this one is used for injecting packets
    PcapTransmitter mPcapTransmitter;
    //RawSocketTransmitter mPcapTransmitter;
    // Used to encrypt the packets
    Encryptor mEncryptor;
    // Used to inject packets
    Ieee80211Header mIeee80211Header;
    // this one never changes,also used to inject packets
    const RadiotapHeader mRadiotapHeader;
    uint16_t ieee80211_seq=0;
    // statistics for console
    // n of packets fed to the instance
    int64_t nInputPackets=0;
    // n of actually injected packets
    int64_t nInjectedPackets=0;
    const std::chrono::steady_clock::time_point INIT_TIME=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point session_key_announce_ts{};
    static constexpr const std::chrono::nanoseconds LOG_INTERVAL=std::chrono::milliseconds(1000);
    Chronometer pcapInjectionTime{"PcapInjectionTime"};
    WBSessionKeyPacket sessionKeyPacket;
    const bool IS_FEC_DISABLED;
    const bool IS_FEC_VARIABLE;
    const FEC_VARIABLE_INPUT_TYPE fecVariableInputType;
    // On the tx, either one of those two is active at the same time
    std::unique_ptr<FECEncoder> mFecEncoder=nullptr;
    std::unique_ptr<FECDisabledEncoder> mFecDisabledEncoder=nullptr;
    std::unique_ptr<std::thread> logAliveThread;
public:
    // run as long as nothing goes completely wrong
    //void loop();
};

#endif //CONSTI10_WIFIBROADCAST_WB_TRANSMITTER_H
