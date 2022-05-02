#include "../src/WBReceiver.h"
#include "../src/HelperSources/SocketHelper.hpp"
#include "../src/HelperSources/SchedulingHelper.hpp"
#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <unistd.h>
#include <pcap/pcap.h>
#include <poll.h>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>

int main(int argc, char *const *argv) {
    int opt;
    ROptions options{};
    int client_udp_port=5600;
    std::string client_addr="127.0.0.1";// default to localhost

    while ((opt = getopt(argc, argv, "K:c:u:r:l:n:k:")) != -1) {
        switch (opt) {
            case 'K':
                options.keypair = optarg;
                break;
            case 'c':
                client_addr = std::string(optarg);
                break;
            case 'u':
                client_udp_port = std::stoi(optarg);
                break;
            case 'r':
                options.radio_port = std::stoi(optarg);
                break;
            case 'l':
                options.log_interval = std::chrono::milliseconds(std::stoi(optarg));
                break;
            case 'k':
            case 'n':
                std::cout<<"-n is deprecated. Please read https://github.com/Consti10/wifibroadcast/blob/master/README.md \n";
                exit(1);
            default: /* '?' */
            show_usage:
                fprintf(stderr,
                        "Local receiver: %s [-K rx_key] [-c client_addr] [-u udp_client_port] [-r radio_port] [-l log_interval(ms)] interface1 [interface2] ...\n",
                        argv[0]);
                fprintf(stderr, "Default: K='%s', connect=%s:%d, radio_port=%d, log_interval=%d \n",
                        "none",client_addr.c_str(), client_udp_port, options.radio_port,
                        (int)std::chrono::duration_cast<std::chrono::milliseconds>(options.log_interval).count());
                fprintf(stderr, "WFB version "
                WFB_VERSION
                "\n");
                exit(1);
        }
    }
    const int nRxInterfaces=argc-optind;
    if(nRxInterfaces>MAX_RX_INTERFACES){
        std::cout<<"Too many RX interfaces "<<nRxInterfaces<<"\n";
        goto show_usage;
    }
    SchedulingHelper::setThreadParamsMaxRealtime();

    //testLol();

    options.rxInterfaces.resize(nRxInterfaces);
    for (int i = 0; i < nRxInterfaces; i++) {
        options.rxInterfaces[i]=std::string(argv[optind + i]);
    }
    try {
        SocketHelper::UDPForwarder udpForwarder(client_addr,client_udp_port);
        std::shared_ptr<WBReceiver> receiver=std::make_shared<WBReceiver>(options, [udpForwarder](const uint8_t* payload, const std::size_t payloadSize){
            udpForwarder.forwardPacketViaUDP(payload,payloadSize);
        });
        receiver->loop();
    } catch (std::runtime_error &e) {
        fprintf(stderr, "Error: %s\n", e.what());
        exit(1);
    }
    return 0;
}
