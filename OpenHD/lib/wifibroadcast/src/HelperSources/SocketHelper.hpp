//
// Created by consti10 on 21.04.22.
//

#ifndef WIFIBROADCAST_SOCKETHELPER_HPP
#define WIFIBROADCAST_SOCKETHELPER_HPP

#include "Helper.hpp"
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <resolv.h>
#include <cstring>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <endian.h>
#include <fcntl.h>
#include <ctime>
#include <sys/mman.h>
#include <string>
#include <vector>
#include <chrono>
#include <cstdarg>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <iostream>
#include <memory>
#include <cassert>
#include <functional>
#include <thread>
#include <algorithm>

namespace SocketHelper{
    static const std::string ADDRESS_LOCALHOST="127.0.0.1";
    static int openUdpSocketForSendingData(const std::string &client_addr, int client_port) {
        struct sockaddr_in saddr;
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw std::runtime_error(StringFormat::convert("Error opening socket: %s", strerror(errno)));

        bzero((char *) &saddr, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = inet_addr(client_addr.c_str());
        saddr.sin_port = htons((unsigned short) client_port);

        if (connect(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
            throw std::runtime_error(StringFormat::convert("Connect error: %s", strerror(errno)));
        }
        return fd;
    }
    // Open the specified port for udp receiving
    // sets SO_REUSEADDR to true if possible
    // throws a runtime exception if opening the socket fails
    static int openUdpSocketForReceiving(const int port){
        int fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (fd < 0) throw std::runtime_error(StringFormat::convert("Error opening socket %d: %s",port, strerror(errno)));
        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
            //throw std::runtime_error(StringFormat::convert("Error setting reuse on socket %d: %s",port, strerror(errno)));
            // don't crash here
            std::cout<<"Cannot set socket reuse\n";
        }
        struct sockaddr_in saddr{};
        bzero((char *) &saddr, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
        saddr.sin_port = htons((unsigned short)port);
        if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0){
            throw std::runtime_error(StringFormat::convert("Bind error on socket %d: %s",port, strerror(errno)));
        }
        return fd;
    }
    // returns the current socket receive timeout
    static std::chrono::nanoseconds getCurrentSocketReceiveTimeout(int socketFd){
        timeval tv{};
        socklen_t len=sizeof(tv);
        auto res=getsockopt(socketFd,SOL_SOCKET,SO_RCVTIMEO,&tv,&len);
        assert(res==0);
        assert(len==sizeof(tv));
        return GenericHelper::timevalToDuration(tv);
    }
    // set the receive timeout on the socket
    // throws runtime exception if this step fails (should never fail on linux)
    static void setSocketReceiveTimeout(int socketFd,const std::chrono::nanoseconds timeout){
        const auto currentTimeout=getCurrentSocketReceiveTimeout(socketFd);
        if(currentTimeout!=timeout){
            //std::cout<<"Changing timeout\n";
            auto tv=GenericHelper::durationToTimeval(timeout);
            if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
                throw std::runtime_error(StringFormat::convert("Cannot set socket timeout %d",timeout.count()));
                //std::cout<<"Cannot set socket timeout "<<timeout.count()<<"\n";
            }
        }
    }
    // Wrapper around an UDP port you can send data to
    // opens port on construction, closes port on destruction
    class UDPForwarder{
    public:
        explicit UDPForwarder(std::string client_addr,int client_udp_port){
            sockfd = SocketHelper::openUdpSocketForSendingData(client_addr, client_udp_port);
            std::cout<<"UDPForwarder::configured for "<<client_addr<<" "<<client_udp_port<<"\n";
        }
        ~UDPForwarder(){
            close(sockfd);
        }
        void forwardPacketViaUDP(const uint8_t *packet,const std::size_t packetSize)const{
            //std::cout<<"Send"<<packetSize<<"\n";
            send(sockfd,packet,packetSize, MSG_DONTWAIT);
        }
    private:
        int sockfd;
    };

    class UDPReceiver{
    public:
        typedef std::function<void(const uint8_t* payload,const std::size_t payloadSize)> OUTPUT_DATA_CALLBACK;
        static constexpr const size_t UDP_PACKET_MAX_SIZE=65507;
        /**
         * Receive data from socket and forward it via callback until stopLooping() is called
         */
        explicit UDPReceiver(std::string client_addr,int client_udp_port,OUTPUT_DATA_CALLBACK cb):mCb(cb){
            mSocket=SocketHelper::openUdpSocketForReceiving(client_udp_port);
            std::cout<<"UDPReceiver created with "<<client_addr<<":"<<client_udp_port<<"\n";
        }
        void loopUntilError(){
            const auto buff=std::make_unique<std::array<uint8_t,UDP_PACKET_MAX_SIZE>>();
            sockaddr_in source;
            socklen_t sourceLen= sizeof(sockaddr_in);
            while (receiving) {
                const ssize_t message_length = recvfrom(mSocket,buff->data(),UDP_PACKET_MAX_SIZE, MSG_WAITALL,(sockaddr*)&source,&sourceLen);
                if (message_length > 0) {
                    mCb(buff->data(), (size_t)message_length);
                }else{
                    std::cout<<"ERROR got message length of:"<<message_length<<"\n";
                    receiving= false;
                }
            }
        }
        void stopLooping(){
            receiving= false;
            close(mSocket);
        }
        void runInBackground(){
            if(receiverThread){
                std::cerr<<"Receiver thread is already running or has not been properly stopped\n";
                return;
            }
            receiverThread=std::make_unique<std::thread>([this](){
                loopUntilError();
            });
        }
        void stopBackground(){
            stopLooping();
            receiverThread->join();
            receiverThread=nullptr;
        }
    private:
        const OUTPUT_DATA_CALLBACK mCb;
        bool receiving=true;
        int mSocket;
        std::unique_ptr<std::thread> receiverThread= nullptr;
    };
}

#endif //WIFIBROADCAST_SOCKETHELPER_HPP
