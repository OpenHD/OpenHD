#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "openhd-log.hpp"
#include "openhd-telemetry.hpp"

#include "control.h"


Control::Control(boost::asio::io_service &io_service): m_udp_socket(io_service) {
    std::cerr << "Control::Control()" << std::endl;
}


void Control::setup() {
    std::cerr << "Control::setup()" << std::endl;

    int control_local_port = 9523;

    m_udp_socket.open(boost::asio::ip::udp::v4());
    m_udp_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), control_local_port));

    start_receive();
}


void Control::start_receive() {
    m_udp_socket.async_receive(boost::asio::buffer(data, max_length),
                               boost::bind(&Control::handle_receive, 
                                           this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
}


void Control::handle_receive(const boost::system::error_code& error,
                             size_t bytes_transferred) {
    if (!error) {
        std::string str(data, data + bytes_transferred);
        command_received(str);
        start_receive();
    }
}
