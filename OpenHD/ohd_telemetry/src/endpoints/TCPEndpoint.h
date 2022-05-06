//
// Created by consti10 on 13.04.22.
//

#ifndef XMAVLINKSERVICE_TCPENDPOINT_H
#define XMAVLINKSERVICE_TCPENDPOINT_H

#include "MEndpoint.hpp"
#include <queue>
#include <vector>
#include <array>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

// Mavlink tcp endpoint (Server)
// Multiple clients can connect to it.
// Supports sending (mavlink) messages to all clients and receiving (mavlink) messages from all clients.
// NOTE: This class purposefully hides away if a client is connected or not. If no client is connected, the
// sendMessage call(s) just returns immediately.
// However, this class will always allow a new client to (re)-connect while running.
class TCPEndpoint: public MEndpoint{
public:
    /**
     * @param Port the port this server runs on
     */
    explicit TCPEndpoint(std::string TAG,int Port);
    void sendMessage(const MavlinkMessage& message) override;
private:
    // The port this server runs on
    const int PORT;
    std::array<uint8_t,1024> readBuffer{};
    boost::asio::io_service _io_service;
    boost::asio::ip::tcp::socket _socket;
    boost::thread allowConnectionThread;
private:
    // try and allow client(s) to connect
    void loopAllowConnection();
    void startReceive();
    void handleRead(const boost::system::error_code& error,
                    size_t bytes_transferred);
};


#endif //XMAVLINKSERVICE_TCPENDPOINT_H
