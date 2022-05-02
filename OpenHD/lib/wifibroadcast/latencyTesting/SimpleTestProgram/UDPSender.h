//
// Created by geier on 28/02/2020.
//

#ifndef LIVEVIDEOSTREAMPRODUCER_UDPSENDER_H
#define LIVEVIDEOSTREAMPRODUCER_UDPSENDER_H

#include <string>
#include <arpa/inet.h>
#include <array>
#include "TimeHelper.hpp"

/**
 * Allows sending UDP data on the current thread. No extra thread for sending is created (make sure to not call mySendTo() on the UI thread)
 */
class UDPSender{
public:
    /**
     * Construct a UDP sender that sends UDP data packets
     * @param IP ipv4 address to send data to
     * @param Port port for sending data
     * @param WANTED_SNDBUFF_SIZE: If not set to 0, increase the buffer allocated by the OS to buffer data before sending UDP packets out
     * Should not increase latency (data is not only sent when this buffer is full)
     */
    UDPSender(const std::string& IP,const int Port,const int WANTED_SNDBUFF_SIZE=0);
    ~UDPSender();
    // Send one udp packet. Packet size must not exceed the max UDP packet size
    // Do not rename to sendto() because this method also exists from the linux socket lib
    // (This method does nothing else than validate the data size, then call sendto()
    void mySendTo(const uint8_t* data, ssize_t data_length);
    //https://en.wikipedia.org/wiki/User_Datagram_Protocol
    //65,507 bytes (65,535 − 8 byte UDP header − 20 byte IP header).
    static constexpr const size_t UDP_PACKET_MAX_SIZE=65507;
    std::size_t nSentBytes=0;
    static constexpr std::size_t EXAMPLE_MEDIUM_SNDBUFF_SIZE=1024*1024;
	void logSendtoDelay();
private:
    int sockfd;
    sockaddr_in address{};
    Chronometer timeSpentSending;
    const int WANTED_SNDBUFF_SIZE;
};


#endif //LIVEVIDEOSTREAMPRODUCER_UDPSENDER_H
