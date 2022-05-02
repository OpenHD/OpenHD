//
// Created by geier on 28/02/2020.
//

#include "UDPSender.h"
#include <cstdlib>
#include <pthread.h>
#include <cerrno>
#include <sys/ioctl.h>
#include <endian.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include "StringHelper.hpp"
#include <iostream>

UDPSender::UDPSender(const std::string &IP,const int Port,const int WANTED_SNDBUFF_SIZE):
        WANTED_SNDBUFF_SIZE(WANTED_SNDBUFF_SIZE)
{
    //create the socket
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0) {
        std::cout<<"Cannot create socket\n";
    }
    //Create the address
    address.sin_family = AF_INET;
    address.sin_port = htons(Port);
    inet_pton(AF_INET,IP.c_str(), &address.sin_addr);
    //
    int sendBufferSize=0;
    socklen_t len=sizeof(sendBufferSize);
    getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, &len);
    std::cout<<"Default socket send buffer is "<<StringHelper::memorySizeReadable(sendBufferSize)<<"\n";
    if(WANTED_SNDBUFF_SIZE!=0){
        if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &WANTED_SNDBUFF_SIZE,len)) {
            std::cout<<"Cannot increase buffer size to "<<StringHelper::memorySizeReadable(WANTED_SNDBUFF_SIZE);
        }
        sendBufferSize=0;
        getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, &len);
        std::cout<<"Wanted "<<StringHelper::memorySizeReadable(WANTED_SNDBUFF_SIZE)<<" Set "<<StringHelper::memorySizeReadable(sendBufferSize)<<"\n";
    }
}

void UDPSender::mySendTo(const uint8_t* data, ssize_t data_length) {
    if(data_length>UDP_PACKET_MAX_SIZE){
        std::cerr<<"Data size exceeds UDP packet size";
        return;
    }
    nSentBytes+=data_length;
    // Measure the time this call takes (is there some funkiness ? )
    timeSpentSending.start();
    const auto result= sendto(sockfd, data, data_length, 0, (struct sockaddr *) &(address),
                                sizeof(struct sockaddr_in));
    if(result<0){
        std::cerr<<"Cannot send data "<<data_length<<" "<<strerror(errno)<<"\n";
    }else{
        //std::cout<<"Sent "<<data_length;
    }
    timeSpentSending.stop();
    //if(timeSpentSending.getNSamples()>100){
        //std::cout<<"TimeSS "<<timeSpentSending.getAvgReadable();
    //    timeSpentSending.reset();
    //}
}
void UDPSender::logSendtoDelay() {
    std::cout<<"Time UDPSender "<<timeSpentSending.getAvgReadable()<<"\n";
}


UDPSender::~UDPSender() {
    //TODO
}
