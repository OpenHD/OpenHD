#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <fmt/core.h>

#include <openhd/mavlink.h>

#include "openhd-status.hpp"
#include "openhd-telemetry.hpp"

#include "router.h"
#include "udpendpoint.h"


void UDPEndpoint::setup(TelemetryType telemetry_type, std::string endpoint_s) {
    std::cerr << "UDPEndpoint::setup(" << endpoint_s << ")" << std::endl;

    m_telemetry_type = telemetry_type;

    boost::smatch result;

    boost::regex r{ "([\\d]+)\\:([\\w\\d\\.]+)\\:([\\d]+)"};
    if (!boost::regex_match(endpoint_s, result, r)) {
        std::cerr << "Failed to match regex" << std::endl;
        return;
    }

    if (result.size() != 4) {
        std::cerr << "Failed size" << std::endl;

        return;
    }

    std::string local_port_s = result[1];
    uint16_t local_port = atoi(local_port_s.c_str());

    std::string address = result[2];
    std::string port_s = result[3];
    uint16_t port = atoi(port_s.c_str());

    std::stringstream _add;

    // we don't know what port will be used yet so we use zero
    _add << fmt::format("{}:{}:{}", "0", address, port_s);

    // this is used by Router to figure out if a dynamic endpoint has been added already
    m_address = _add.str();

    m_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_udp_socket.open(boost::asio::ip::udp::v4());
    m_udp_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), local_port));

    start_receive();
}


void UDPEndpoint::start_receive() {
    m_udp_socket.async_receive_from(boost::asio::buffer(data, max_length),
                                    m_endpoint,
                                    boost::bind(&UDPEndpoint::handle_read, 
                                                this,
                                                boost::asio::placeholders::error, 
                                                boost::asio::placeholders::bytes_transferred));
}


void UDPEndpoint::handle_read(const boost::system::error_code& error,
                              size_t bytes_transferred) {
    if (!error) {
        switch (m_telemetry_type) {
            case TelemetryTypeMavlink: {
                mavlink_message_t msg;
                for (int i = 0; i < bytes_transferred; i++) {
                    uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)data[i], &msg, &m_mavlink_status);
                    if (res) {
                        if (msg.sysid != 0) {
                            add_known_sys_id(msg.sysid);
                        }
                        m_router->process_mavlink_message(shared_from_this(), msg);
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
        start_receive();
    }
}


void UDPEndpoint::send_message(uint8_t *buffer, int size) {
    auto sent = m_udp_socket.send_to(boost::asio::buffer(buffer, size), m_endpoint);
}