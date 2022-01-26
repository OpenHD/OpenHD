
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

struct Options{
    // the radio port is what is used as an index to multiplex multiple streams (telemetry,video,...)
    // into the one wfb stream
    uint8_t radio_port = 1;
    // input UDP port
    int udp_port = 5600;
    // file for encryptor
    // make optional for ease of use - with no keypair given the default "seed" is used
    // std::string keypair="drone.key";
    std::optional<std::string> keypair=std::nullopt;
    // wlan interface to send packets with
    std::string wlan;
    // either fixed or variable. If int==fixed, if string==variable but hook needs to be added (currently only hooked h264 and h265)
    std::variant<int,std::string> fec_k=8;
    int fec_percentage=50;
};
enum FEC_VARIABLE_INPUT_TYPE{none,h264,h265};

// WBTransmitter uses an UDP port as input for the data stream
// Each input UDP port has to be assigned with a Unique ID to differentiate between streams on the RX
// It does all the FEC encoding & encryption for this stream, then uses PcapTransmitter to inject the generated packets
// FEC can be either enabled or disabled.
class WBTransmitter {
public:
    WBTransmitter(RadiotapHeader radiotapHeader,const Options& options1);
    ~WBTransmitter();
private:
    const Options& options;
    // process the input data stream
    void processInputPacket(const uint8_t *buf, size_t size);
    // send the current session key via WIFI (located in mEncryptor)
    void sendSessionKey();
    // for the FEC encoder
    void sendFecPrimaryOrSecondaryFragment(const uint64_t nonce, const uint8_t* payload,const size_t payloadSize);
    // send packet by prefixing data with the current IEE and Radiotap header
    void sendPacket(const AbstractWBPacket& abstractWbPacket);
    // this one is used for injecting packets
    PcapTransmitter mPcapTransmitter;
    //RawSocketTransmitter mPcapTransmitter;
    // the rx socket is set by opening the right UDP port
    int mInputSocket;
    // Used to encrypt the packets
    Encryptor mEncryptor;
    // Used to inject packets
    Ieee80211Header mIeee80211Header;
    // this one never changes,also used to inject packets
    const RadiotapHeader mRadiotapHeader;
    uint16_t ieee80211_seq=0;
    // statistics for console
    int64_t nPacketsFromUdpPort=0;
    int64_t nInjectedPackets=0;
    const std::chrono::steady_clock::time_point INIT_TIME=std::chrono::steady_clock::now();
    static constexpr const std::chrono::nanoseconds LOG_INTERVAL=std::chrono::milliseconds(1000);
    Chronometer pcapInjectionTime{"PcapInjectionTime"};
    WBSessionKeyPacket sessionKeyPacket;
    const bool IS_FEC_DISABLED;
    const bool IS_FEC_VARIABLE;
    const FEC_VARIABLE_INPUT_TYPE fecVariableInputType;
    // On the tx, either one of those two is active at the same time
    std::unique_ptr<FECEncoder> mFecEncoder=nullptr;
    std::unique_ptr<FECDisabledEncoder> mFecDisabledEncoder=nullptr;
public:
    // run as long as nothing goes completely wrong
    void loop();
};
