#ifndef CONTROL_H
#define CONTROL_H

#include <array>
#include <vector>

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>

#include "endpoint.h"

class Router;


class Control {
public:
    Control(Router *router, boost::asio::io_service &io_service, bool is_microservice);

    void setup();

    void start_receive();

protected:
    void handle_receive(const boost::system::error_code& error,
                        size_t bytes_transferred);

    Router *m_router = nullptr;

    bool m_is_microservice = false;

    enum { max_length = 1024 };
    char data[max_length];

    boost::asio::ip::udp::socket m_udp_socket;
};

#endif // CONTROL_H
