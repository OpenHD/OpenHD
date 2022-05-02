#ifndef CONTROL_H
#define CONTROL_H

#include <array>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/signals2.hpp>


class Control {
public:
    Control(boost::asio::io_service &io_service);

    void setup();

    void start_receive();

    boost::signals2::signal<void (std::string command_uri)> command_received;

protected:
    void handle_receive(const boost::system::error_code& error,
                        size_t bytes_transferred);

    enum { max_length = 1024 };
    char data[max_length];

    boost::asio::ip::udp::socket m_udp_socket;
};

#endif // CONTROL_H
