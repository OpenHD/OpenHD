#include "../src/WBTransmitter.h"
#include "../src/HelperSources/SocketHelper.hpp"
#include "../src/HelperSources/SchedulingHelper.hpp"
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


int main(int argc, char *const *argv) {
    int opt;
    TOptions options{};
    // input UDP port
    int udp_port = 5600;

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
                udp_port = std::stoi(optarg);
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
                        "none", std::get<int>(options.fec_k), options.fec_percentage, udp_port, options.radio_port, wifiParams.bandwidth, wifiParams.short_gi ? "short" : "long", wifiParams.stbc, wifiParams.ldpc, wifiParams.mcs_index);
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
        SocketHelper::UDPReceiver udpReceiver(SocketHelper::ADDRESS_LOCALHOST,udp_port,[t](const uint8_t* payload,const std::size_t payloadSize){
            t->feedPacket(payload,payloadSize);
        });
        udpReceiver.loopUntilError();
    } catch (std::runtime_error &e) {
        fprintf(stderr, "Error: %s\n", e.what());
        exit(1);
    }
    return 0;
}
