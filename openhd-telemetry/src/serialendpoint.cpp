#include <algorithm>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "openhd-status.hpp"

#include "router.h"
#include "serialendpoint.h"



void SerialEndpoint::setup(TelemetryType telemetry_type, std::string endpoint_s) {
    std::cerr << "SerialEndpoint::setup(" << endpoint_s << ")" << std::endl;

    m_telemetry_type = telemetry_type;

    boost::smatch result;

    boost::regex r{ "([\\/\\w\\d]+)\\:([\\w\\d]+)"};

    if (!boost::regex_match(endpoint_s, result, r)) {
        status_message(STATUS_LEVEL_EMERGENCY, "Serial endpoint setting not in proper format");
        return;
    }

    if (result.size() != 3) {
        status_message(STATUS_LEVEL_EMERGENCY, "Serial endpoint setting not in proper format");
        return;
    }


    std::string serial_port = result[1];

    std::string baud_s = result[2];
    int baud = atoi(baud_s.c_str());


    try {
        std::cerr << "Opening serial port: " << serial_port << " baud: " << baud_s << std::endl;
        m_serial.open(serial_port);
    } catch (boost::system::system_error::exception e) {
        status_message(STATUS_LEVEL_EMERGENCY, "Failed to open serial port");
        return;
    }


    try {
        m_serial.set_option(boost::asio::serial_port_base::baud_rate(baud));
        m_serial.set_option(boost::asio::serial_port_base::character_size(8));
        m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    } catch (boost::system::system_error::exception e) {
        status_message(STATUS_LEVEL_EMERGENCY, "Faild to set serial port baud rate");
        return;
    }


    start_receive();
}


void SerialEndpoint::send_message(uint8_t* buf, size_t size) {
    if (!m_serial.is_open()) {
        return;
    }

    boost::asio::async_write(m_serial,
                             boost::asio::buffer(buf, size),
                             boost::bind(&SerialEndpoint::handle_serial_write,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}


void SerialEndpoint::start_receive() {
    m_serial.async_read_some(boost::asio::buffer(data, max_length),
                             boost::bind(&SerialEndpoint::handle_serial_read,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}


void SerialEndpoint::handle_serial_write(const boost::system::error_code& error,
                                         size_t bytes_transferred) {}


void SerialEndpoint::handle_serial_read(const boost::system::error_code& error,
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
    }

    start_receive();
}