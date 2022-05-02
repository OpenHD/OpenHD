//
// Created by consti10 on 15.10.21.
//
#include "TimeHelper.hpp"
#include "Helper.hpp"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include "StringHelper.hpp"
#include "SchedulingHelper.hpp"
#include <iostream>
#include <cstring>
#include <atomic>
#include <mutex>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>

// Forward udp data from localhost(ip):port to destination(ip):port

static bool quit = false;
static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    quit = true;
}

static std::unique_ptr<UDPSender> udpSender= nullptr;

static void forwardData(const uint8_t* dataP,size_t data_length){
    udpSender->mySendTo(dataP,data_length);
    std::cout<<"Forwarded data "<<data_length<<"B\n";
}


int main(int argc, char *argv[])
{
    // input is localhost:port
    int inputPort=6000;
    // output is ip:port
    std::string outputIp="192.168.0.14";
    int outputPort=6001;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:x:")) != -1) {
        switch (opt) {
            case 'i':{
                inputPort=std::stoi(optarg);
            };
                break;
            case 'o':{
                outputIp=optarg;
            };
                break;
            case 'x':{
                outputPort=std::stoi(optarg);
            };
                break;
            default:
                std::cout<<"Usage: -i input_port -o output_ip -x output_port\n";
                return 0;
        }
    }

    // create the sender first
    udpSender=std::make_unique<UDPSender>(outputIp,outputPort);

    UDPReceiver udpReceiver{inputPort,"ForwardRec", forwardData,0,false};
    udpReceiver.startReceiving();

    std::cout<<"Now forwarding from localhost:"<<inputPort<<" to "<<outputIp<<":"<<outputPort<<"\n";

    signal(SIGINT, sigterm_handler);
    while (!quit) {
        usleep(100);
    }

    udpReceiver.stopReceiving();

    return 0;
}
