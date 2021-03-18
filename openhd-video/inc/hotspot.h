#ifndef HOTSPOT_H
#define HOTSPOT_H

#include <array>
#include <chrono>
#include <vector>
#include <stdexcept>


#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"

#include "openhd-platform.hpp"

class Hotspot {
public:
    Hotspot(boost::asio::io_service &io_service, uint16_t port);
    
    void setup();

    void command_received(std::string command_uri);

private:
    boost::asio::io_service &m_io_service;

    uint16_t m_port = 0;

    std::vector<std::string> m_endpoints;

    void start_receive();

    void handle_receive(const boost::system::error_code& error,
                        size_t bytes_transferred);

    enum { max_length = 65535 };
    char data[max_length];

    boost::asio::ip::udp::socket m_udp_socket;
};


#endif

