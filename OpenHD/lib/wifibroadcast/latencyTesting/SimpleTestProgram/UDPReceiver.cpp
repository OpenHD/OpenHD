
#include "UDPReceiver.h"
#include <arpa/inet.h>
#include <utility>
#include <vector>
#include <sstream>
#include <array>
#include "StringHelper.hpp"
#include <cstring>

#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>

UDPReceiver::UDPReceiver(int port,std::string name,DATA_CALLBACK  onDataReceivedCallback,
size_t WANTED_RCVBUF_SIZE,const bool ENABLE_NONBLOCKING):
        mPort(port),mName(std::move(name)),WANTED_RCVBUF_SIZE(WANTED_RCVBUF_SIZE),onDataReceivedCallback(std::move(onDataReceivedCallback)),ENABLE_NONBLOCKING(ENABLE_NONBLOCKING){
}

long UDPReceiver::getNReceivedBytes()const {
    return nReceivedBytes;
}

std::string UDPReceiver::getSourceIPAddress()const {
    return senderIP;
}

void UDPReceiver::startReceiving() {
    receiving=true;
    mUDPReceiverThread=std::make_unique<std::thread>([this]{this->receiveFromUDPLoop();} );
}

void UDPReceiver::stopReceiving() {
    receiving=false;
    //this stops the recvfrom even if in blocking mode
    shutdown(mSocket,SHUT_RD);
    if(mUDPReceiverThread->joinable()){
        mUDPReceiverThread->join();
    }
    mUDPReceiverThread.reset();
	std::cout<<"UDPReceiver avgDeltaBetween(recvfrom) "<<avgDeltaBetweenPackets.getAvgReadable()<<"\n";
}

void UDPReceiver::receiveFromUDPLoop() {
    mSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mSocket == -1) {
        std::cout<<"Error creating socket\n";
        return;
    }
    int enable = 1;
    if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        std::cout<<"Error setting reuse\n";
    }
    //if(setsockopt(mSocket,SOL_SOCKET,SO_REUSEPORT,&enable,sizeof(int))<0){
    //    std::cout<<"Error setting SO_REUSEPORT\n";
    //}

    int recvBufferSize=0;
    socklen_t len=sizeof(recvBufferSize);
    getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, &len);
    std::cout<<"Default socket recv buffer is "<<StringHelper::memorySizeReadable(recvBufferSize)<<"\n";

    if(WANTED_RCVBUF_SIZE>recvBufferSize){
        recvBufferSize=WANTED_RCVBUF_SIZE;
        if(setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, &WANTED_RCVBUF_SIZE,len)) {
            std::cout<<"Cannot increase buffer size to "<<StringHelper::memorySizeReadable(WANTED_RCVBUF_SIZE)<<"\n";
        }
        getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, &len);
        std::cout<<"Wanted "<<StringHelper::memorySizeReadable(WANTED_RCVBUF_SIZE)<<" Set "<<StringHelper::memorySizeReadable(recvBufferSize)<<"\n";
    }
    struct sockaddr_in myaddr;
    memset((uint8_t *) &myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(mPort);
    if (bind(mSocket, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        std::cerr<<"Error binding Port; "<<mPort<<"\n";
        return;
    }
    //wrap into unique pointer to avoid running out of stack
    const auto buff=std::make_unique<std::array<uint8_t,UDP_PACKET_MAX_SIZE>>();

    sockaddr_in source;
    socklen_t sourceLen= sizeof(sockaddr_in);
    
    while (receiving) {
        //TODO investigate: does a big buffer size create latency with MSG_WAITALL ?
        //I do not think so. recvfrom should return as soon as new data arrived,not when the buffer is full
        //But with a bigger buffer we do not loose packets when the receiver thread cannot keep up for a short amount of time
        // MSG_WAITALL does not wait until we have __n data, but a new UDP packet (that can be smaller than __n)
        //NOTE: NONBLOCKING hogs a whole CPU core ! do not use whenever possible !
		ssize_t tmp;
		if(ENABLE_NONBLOCKING){
			tmp = recvfrom(mSocket,buff->data(),UDP_PACKET_MAX_SIZE, MSG_DONTWAIT,(sockaddr*)&source,&sourceLen);
		}else{
			tmp = recvfrom(mSocket,buff->data(),UDP_PACKET_MAX_SIZE, MSG_WAITALL,(sockaddr*)&source,&sourceLen);
		}
		const ssize_t message_length=tmp;
        if (message_length > 0) { //else -1 was returned;timeout/No data received
			if(lastReceivedPacket!=std::chrono::steady_clock::time_point{}){
				const auto delta=std::chrono::steady_clock::now()-lastReceivedPacket;
				avgDeltaBetweenPackets.add(delta);
			}
			lastReceivedPacket=std::chrono::steady_clock::now();
            //LOGD("Data size %d",(int)message_length);
            onDataReceivedCallback(buff->data(), (size_t)message_length);

            nReceivedBytes+=message_length;
            //The source ip stuff
            const char* p=inet_ntoa(source.sin_addr);
            std::string s1=std::string(p);
            if(senderIP!=s1){
                senderIP=s1;
            }
        }else{
            if(errno != EWOULDBLOCK) {
                //MLOGE<<"Error on recvfrom. errno="<<errno<<" "<<strerror(errno);
            }
        }
    }
    close(mSocket);
}

int UDPReceiver::getPort() const {
    return mPort;
}
