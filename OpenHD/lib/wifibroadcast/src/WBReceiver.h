#ifndef CONSTI10_WIFIBROADCAST_WB_RECEIVER_H
#define CONSTI10_WIFIBROADCAST_WB_RECEIVER_H
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
#include "wifibroadcast.hpp"
#include "Encryption.hpp"
#include "FECEnabled.hpp"
#include "FECDisabled.hpp"
#include "HelperSources/Helper.hpp"
#include "OpenHDStatisticsWriter.hpp"
#include "HelperSources/TimeHelper.hpp"
#include "RawReceiver.hpp"

#include <unordered_map>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>
#include <utility>

// A wifi card with more than 4 antennas still has to be found :)
static constexpr const auto MAX_N_ANTENNAS_PER_WIFI_CARD=4;
//

struct ROptions{
    uint8_t radio_port=0;
    // The wlan adapters to listen on
    std::vector<std::string> rxInterfaces;
    // file for encryptor
    // make optional for ease of use - with no keypair given the default "seed" is used
    std::optional<std::string> keypair=std::nullopt;
    // allows setting the log interval
    std::chrono::milliseconds log_interval;
};

class WBReceiver{
public:
    typedef std::function<void(const uint8_t* payload,const std::size_t payloadSize)> OUTPUT_DATA_CALLBACK;
    /**
     * This class processes the received wifi raw wifi data
     * (aggregation, FEC decoding) and forwards it via the callback.
     * Each instance has to be assigned with a Unique ID (same id as the corresponding tx instance).
     * @param options1 the options for this instance (some options - so to say - come from the tx instance)
     * @param callback Callback that is called with the decoded data, can be null for debugging.
     */
    explicit WBReceiver(const ROptions& options1,OUTPUT_DATA_CALLBACK callback);
    ~WBReceiver()=default;
    void processPacket(uint8_t wlan_idx,const pcap_pkthdr& hdr,const uint8_t* pkt);
    // dump statistics
    void dump_stats();
    const ROptions& options;
    /**
     * Process incoming data packets as long as nothing goes wrong (nothing should go wrong as long
     * as the computer does not crash or the wifi card disconnects).
     * NOTE: This class won't receive any messages until loop() is called
     */
    void loop();
private:
    const std::chrono::steady_clock::time_point INIT_TIME=std::chrono::steady_clock::now();
    Decryptor mDecryptor;
    std::array<RSSIForWifiCard,MAX_RX_INTERFACES> rssiForWifiCard;
    // n of all received packets, absolute
    uint64_t count_p_all=0;
    // n of received packets that are bad for any reason
    uint64_t count_p_bad=0;
    // encryption stats
    uint64_t count_p_decryption_err=0;
    uint64_t count_p_decryption_ok=0;
    OpenHDStatisticsWriter openHdStatisticsWriter;
    //We know that once we get the first session key packet
    bool IS_FEC_ENABLED=false;
    // On the rx, either one of those two is active at the same time. NOTE: nullptr until the first session key packet
    std::unique_ptr<FECDecoder> mFECDDecoder=nullptr;
    std::unique_ptr<FECDisabledDecoder> mFECDisabledDecoder=nullptr;
    //Ieee80211HeaderSeqNrCounter mSeqNrCounter;
    // Callback that is called with the decoded data
    const OUTPUT_DATA_CALLBACK mOutputDataCallback;
    std::unique_ptr<MultiRxPcapReceiver> receiver;
public:
#ifdef ENABLE_ADVANCED_DEBUGGING
    // time between <packet arrives at pcap processing queue> <<->> <packet is pulled out of pcap by RX>
    AvgCalculator avgPcapToApplicationLatency;
    AvgCalculator2 avgLatencyBeaconPacketLatency;
#endif
};

#endif //CONSTI10_WIFIBROADCAST_WB_RECEIVER_H