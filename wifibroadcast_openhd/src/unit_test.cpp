
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

#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <ctime>
#include <climits>

#include <memory>
#include <string>
#include <chrono>
#include <sstream>

#include "wifibroadcast.hpp"
#include "FECEnabled.hpp"

#include "HelperSources/Helper.hpp"
#include "Encryption.hpp"

// Simple unit testing for the FEC lib that doesn't require wifi cards

namespace TestFEC{

    static void testNonce(){
        const uint32_t blockIdx=0;
        const uint16_t fragmentIdx=0;
        const uint16_t number=1;
        const FECNonce fecNonce{blockIdx,fragmentIdx,false,number};
        const auto nonce=(uint64_t)fecNonce;
        const auto fecNonce2=fecNonceFrom(nonce);
        assert(fecNonce2.blockIdx==blockIdx);
        assert(fecNonce2.fragmentIdx==fragmentIdx);
        assert(fecNonce2.flag==0);
        assert(fecNonce2.number==number);
    }

    // test without packet loss, fixed block size
    static void testWithoutPacketLoss(const int k, const int percentage, const std::vector<std::vector<uint8_t>>& testIn){
        std::cout<<"Test without packet loss. K:"<<k<<" P:"<<percentage<<" N_PACKETS:"<<testIn.size()<<"\n";
        FECEncoder encoder(k,percentage);
        FECDecoder decoder;
        std::vector<std::vector<uint8_t>> testOut;

        const auto cb1=[&decoder](const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)mutable {
            decoder.validateAndProcessPacket(nonce, std::vector<uint8_t>(payload,payload +payloadSize));
        };
        const auto cb2=[&testOut](const uint8_t * payload,std::size_t payloadSize)mutable{
            testOut.emplace_back(payload,payload+payloadSize);
        };
        encoder.outputDataCallback=cb1;
        decoder.mSendDecodedPayloadCallback=cb2;
        // If there is no data loss the packets should arrive immediately
        for(std::size_t i=0;i<testIn.size();i++){
            //std::cout<<"Step\n";
            const auto& in=testIn[i];
            encoder.encodePacket(in.data(),in.size());
            const auto& out=testOut[i];
            assert(GenericHelper::compareVectors(in,out)==true);
        }
    }
    // test without packet loss, dynamic block size aka
    // randomly end the block at some time
    static void testWithoutPacketLossDynamicBlockSize(){
        std::cout<<"Test without packet loss dynamic block size\n";
        constexpr auto N_BLOCKS=2000;
        const auto testIn=GenericHelper::createRandomDataBuffers(N_BLOCKS, FEC_MAX_PAYLOAD_SIZE, FEC_MAX_PAYLOAD_SIZE);
        std::vector<std::vector<uint8_t>> testOut;
        FECEncoder encoder(MAX_N_P_FRAGMENTS_PER_BLOCK,50);
        FECDecoder decoder;
        const auto cb1=[&decoder](const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)mutable {
            decoder.validateAndProcessPacket(nonce, std::vector<uint8_t>(payload,payload +payloadSize));
        };
        const auto cb2=[&testOut](const uint8_t * payload,std::size_t payloadSize)mutable{
            testOut.emplace_back(payload,payload+payloadSize);
        };
        encoder.outputDataCallback=cb1;
        decoder.mSendDecodedPayloadCallback=cb2;
        for(std::size_t i=0;i<testIn.size();i++){
            //std::cout<<"Step\n";
            const bool endBlock=(rand() % 10)==0;
            const auto& in=testIn[i];
            encoder.encodePacket(in.data(),in.size(),endBlock);
            const auto& out=testOut[i];
            assert(GenericHelper::compareVectors(in,out)==true);
        }
    }
    // Put packets in in such a order that the rx queue is tested
    static void testRxQueue(const int k, const int percentage){
        std::cout<<"Test rx queue. K:"<<k<<" P:"<<percentage<<"\n";
        const auto n=FECEncoder::calculateN(k,percentage);
        constexpr auto QUEUE_SIZE=FECDecoder::RX_QUEUE_MAX_SIZE;
        const auto testIn=GenericHelper::createRandomDataBuffers(QUEUE_SIZE*k, FEC_MAX_PAYLOAD_SIZE, FEC_MAX_PAYLOAD_SIZE);
        FECEncoder encoder(k,percentage);
        FECDecoder decoder;
        // begin test
        std::vector<std::pair<uint64_t,std::vector<uint8_t>>> fecPackets;
        const auto cb1=[&fecPackets](const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)mutable {
            fecPackets.emplace_back(nonce,std::vector<uint8_t>(payload,payload +payloadSize));
        };
        encoder.outputDataCallback=cb1;
        // process all input packets
        for(const auto& in:testIn){
            encoder.encodePacket(in.data(),in.size());
        }
        // now add them to the decoder (queue):
        std::vector<std::vector<uint8_t>> testOut;
        const auto cb2=[&testOut](const uint8_t * payload,std::size_t payloadSize)mutable{
            testOut.emplace_back(payload,payload+payloadSize);
        };
        decoder.mSendDecodedPayloadCallback=cb2;
        // add fragments (primary fragments only to not overcomplicate things)
        // but in the following order:
        // block 0, fragment 0, block 1, fragment 0, block 2, fragment 0, ... until block X, fragment n
        for(int frIdx=0; frIdx < k; frIdx++){
            for(int i=0;i<QUEUE_SIZE;i++){
                const auto idx=i*n + frIdx;
                std::cout<<"adding"<<idx<<"\n";
                const auto& packet=fecPackets.at(idx);
                decoder.validateAndProcessPacket(packet.first,packet.second);
            }
        }
        // and then check if in and out match
        for(std::size_t i=0;i<testIn.size();i++){
            //std::cout<<"Step\n";
            const auto& in=testIn[i];
            const auto& out=testOut[i];
            GenericHelper::assertVectorsEqual(in,out);
        }
    }

    // No packet loss
    // Fixed packet size
    static void testWithoutPacketLossFixedPacketSize(const int k,const int percentage, const std::size_t N_PACKETS){
        auto testIn=GenericHelper::createRandomDataBuffers(N_PACKETS, FEC_MAX_PAYLOAD_SIZE, FEC_MAX_PAYLOAD_SIZE);
        testWithoutPacketLoss(k, percentage, testIn);
    }

    // No packet loss
    // Dynamic packet size (up to N bytes)
    static void testWithoutPacketLossDynamicPacketSize(const int k,const int percentage, const std::size_t N_PACKETS){
        auto testIn=GenericHelper::createRandomDataBuffers(N_PACKETS, 1, FEC_MAX_PAYLOAD_SIZE);
        testWithoutPacketLoss(k, percentage, testIn);
    }

    // test with packet loss
    // but only drop as much as everything must be still recoverable
    static void testWithPacketLossButEverythingIsRecoverable(const int k,const unsigned int percentage, const std::vector<std::vector<uint8_t>>& testIn,const int DROP_MODE,const bool SEND_DUPLICATES=false) {
        assert(testIn.size() % k==0);
        const auto n=FECEncoder::calculateN(k,percentage);
        // drop mode 2 is impossible if (n-k)<2
        if(DROP_MODE==2){
            //assert((k*percentage/100)>=2);
            assert((n-k)>=2);
        }
        std::cout << "Test (with packet loss) K:" << k << " P:" << percentage << " N_PACKETS:" << testIn.size() <<" DROP_MODE:"<<DROP_MODE<< "\n";
        FECEncoder encoder(k,percentage);
        FECDecoder decoder;
        std::vector <std::vector<uint8_t>> testOut;
        const auto cb1 = [&decoder,k,DROP_MODE,SEND_DUPLICATES](const uint64_t nonce,const uint8_t* payload,const std::size_t payloadSize)mutable {
            const FECNonce fecNonce=fecNonceFrom(nonce);
            const auto blockIdx=fecNonce.blockIdx;
            const auto fragmentIdx=fecNonce.fragmentIdx;
            if(DROP_MODE==0){
                // drop all FEC correction packets but no data packets (everything should be still recoverable
                if(fragmentIdx>=k){
                    std::cout<<"Dropping FEC-CORRECTION packet:["<<blockIdx<<","<<(int)fragmentIdx<<"]\n";
                    return;
                }
            }else if(DROP_MODE==1){
                // drop 1 data packet and let FEC do its magic
                if(fragmentIdx==0){
                    std::cout<<"Dropping FEC-DATA packet:["<<blockIdx<<","<<(int)fragmentIdx<<"]\n";
                    return;
                }
            }else if(DROP_MODE==2){
                // drop 1 data packet and 1 FEC packet but that still shouldn't pose any issues
                if(fragmentIdx==0){
                    std::cout<<"Dropping FEC-DATA packet:["<<blockIdx<<","<<(int)fragmentIdx<<"]\n";
                    return;
                }else if(fragmentIdx==k-1){
                    std::cout<<"Dropping FEC-CORRECTION packet:["<<blockIdx<<","<<(int)fragmentIdx<<"]\n";
                    return;
                }
            }
            if(SEND_DUPLICATES){
                // emulate not more than N multiple wifi cards as rx
                const auto duplicates=std::rand() % 8;
                for(int i=0;i<duplicates+1;i++){
                    decoder.validateAndProcessPacket(nonce,
                                                     std::vector<uint8_t>(payload, payload +payloadSize));
                }
            }else{
                decoder.validateAndProcessPacket(nonce, std::vector<uint8_t>(payload,payload + payloadSize));
            }
        };
        const auto cb2 = [&testOut](const uint8_t *payload, std::size_t payloadSize)mutable {
            testOut.emplace_back(payload, payload + payloadSize);
        };
        encoder.outputDataCallback = cb1;
        decoder.mSendDecodedPayloadCallback = cb2;
        for (std::size_t i = 0; i < testIn.size(); i++) {
            const auto &in = testIn[i];
            encoder.encodePacket(in.data(), in.size());
            // every time we have sent enough packets to form a block, check if everything arrived
            // This way we would also catch any unwanted latency created by the decoder as an error
            if(i % k ==0 && i>0){
                for(std::size_t j=0;j<i;j++){
                    assert(GenericHelper::compareVectors(testIn[j], testOut[j]) == true);
                }
            }
        }
        // just to be sure, check again
        assert(testIn.size()==testOut.size());
        for (std::size_t i = 0; i < testIn.size(); i++) {
            const auto &in = testIn[i];
            const auto &out = testOut[i];
            assert(GenericHelper::compareVectors(in, out) == true);
        }
    }

    static void testWithPacketLossButEverythingIsRecoverable(const int k, const unsigned int percentage, const std::size_t N_PACKETS, const int DROP_MODE){
        std::vector<std::vector<uint8_t>> testIn;
        for(std::size_t i=0;i<N_PACKETS;i++){
            const auto size= (rand() % FEC_MAX_PAYLOAD_SIZE) + 1;
            testIn.push_back(GenericHelper::createRandomDataBuffer(size));
        }
        testWithPacketLossButEverythingIsRecoverable(k, percentage, testIn,DROP_MODE, false);
    }
}

namespace TestEncryption{
    static void test(const bool useGeneratedFiles){
        std::cout<<"Using generated keypair (default seed otherwise):"<<(useGeneratedFiles ? "y":"n")<<"\n";
        std::optional<std::string> encKey=useGeneratedFiles ?  std::optional<std::string>("gs.key") : std::nullopt;
        std::optional<std::string> decKey=useGeneratedFiles ?  std::optional<std::string>("drone.key") : std::nullopt;

        Encryptor encryptor{encKey};
        Decryptor decryptor{decKey};
        WBSessionKeyPacket sessionKeyPacket;
        // make session key (tx)
        encryptor.makeNewSessionKey(sessionKeyPacket.sessionKeyNonce, sessionKeyPacket.sessionKeyData);
        // and "receive" session key (rx)
        assert(decryptor.onNewPacketSessionKeyData(sessionKeyPacket.sessionKeyNonce, sessionKeyPacket.sessionKeyData) == true);
        // now encrypt a couple of packets and decrypt them again afterwards
        for(uint64_t nonce=0; nonce < 20; nonce++){
            const auto data=GenericHelper::createRandomDataBuffer(FEC_MAX_PAYLOAD_SIZE);
            const WBDataHeader wbDataHeader(nonce);
            const auto encrypted=encryptor.encryptPacket(wbDataHeader.nonce,data.data(),data.size(),wbDataHeader);
            const auto decrypted=decryptor.decryptPacket(wbDataHeader.nonce,encrypted.data(), encrypted.size(),wbDataHeader);
            assert(decrypted!=std::nullopt);
            assert(GenericHelper::compareVectors(data,*decrypted) == true);
        }
        std::cout<<"encryption test passed\n";
    }
}


int main(int argc, char *argv[]){
    std::cout<<"Tests for Wifibroadcast\n";
    srand (time(NULL));
    int opt;
    int test_mode=0;

    while ((opt = getopt(argc, argv, "m:")) != -1) {
        switch (opt) {
            case 'm':
                test_mode = atoi(optarg);
                break;
            default: /* '?' */
            show_usage:
                std::cout<<"Usage: Unit tests for FEC and encryption. -m 0,1,2 test mode: 0==ALL, 1==FEC only 2==Encryption only\n";
                return 1;
        }
    }


    try {
        if(test_mode==0 || test_mode==1){
            std::cout<<"Testing FEC\n";
            testFecCPlusPlusWrapperX();
            const int N_PACKETS=1200;
            TestFEC::testNonce();
            // With these fec params "testWithoutPacketLoss" is not possible
            const std::vector<std::pair<unsigned int,unsigned int>> fecParams1={
                    {1,0},{1,100},
                    {2,0},{2,50},{2,100}
            };
            for(const auto& fecParam:fecParams1){
                const auto k=fecParam.first;
                const auto p=fecParam.second;
                TestFEC::testWithoutPacketLossFixedPacketSize(k,p, N_PACKETS);
                TestFEC::testWithoutPacketLossFixedPacketSize(k,p, N_PACKETS);
            }
            // only test with FEC enabled
            const std::vector<std::pair<unsigned int,unsigned int>> fecParams={
                    {1,200},
                    {2,100},{2,200},
                    {4,100},{4,200},
                    {6,50},{6,100},{6,200},
                    {8,50},{8,100},{8,200},
                    {10,30},{10,50},{10,100},
                    {40,30},{40,50},{40,100},
                    {100,30},{100,40},{100,50},{100,60},
                    {120,50}
            };
            for(const auto& fecParam:fecParams){
                const auto k=fecParam.first;
                const auto p=fecParam.second;
                TestFEC::testWithoutPacketLossFixedPacketSize(k, p, N_PACKETS);
                TestFEC::testWithoutPacketLossDynamicPacketSize(k, p, N_PACKETS);
                TestFEC::testRxQueue(k, p);
                for(int dropMode=1;dropMode<2;dropMode++){
                    TestFEC::testWithPacketLossButEverythingIsRecoverable(k, p, N_PACKETS, dropMode);
                }
            }
            TestFEC::testWithoutPacketLossDynamicBlockSize();
        }
        if(test_mode==0 || test_mode==2){
            //
            std::cout<<"Testing Encryption\n";
            TestEncryption::test(false);
            TestEncryption::test(true);
            //
        }
    }catch (std::runtime_error &e) {
        std::cerr<<"Error: "<<std::string(e.what());
        exit(1);
    }
    std::cout<<"All Tests Passing\n";
    return 0;
}