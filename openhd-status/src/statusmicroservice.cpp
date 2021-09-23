#include "../inc/statusmicroservice.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/reboot.h>

#include <iostream>
#include <fstream>
#include <streambuf>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <openhd/mavlink.h>

#include "openhd-platform.hpp"

#include "../../openhd-common/openhd-status.hpp"



constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER3;

std::string get_openhd_version() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("dpkg-query --showformat='${Version}' --show openhd", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Checking openhd version failed");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


StatusMicroservice::StatusMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id):
		Microservice(io_service, platform), m_udp_socket(io_service), m_is_air(is_air), m_unit_id(unit_id) {
    set_compid(SERVICE_COMPID);

    m_udp_socket.open(boost::asio::ip::udp::v4());
    m_udp_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 50000));
}


void StatusMicroservice::setup() {
    std::cout << "StatusMicroservice::setup()" << std::endl;

    try {
        m_openhd_version = get_openhd_version();
    } catch (std::exception& ex) {
        // the exception itself doesn't matter, will just mean the m_openhd_version default
        // gets sent to QOpenHD
    }

    /*
     * The timing at early boot is fairly strict, we need OpenHDBoot to quickly
     * connect to the status service and start displaying messages, which requires
     * receiving heartbeat and sys_time messages as fast as possible. OpenHDBoot will then
     * immediately send OPENHD_CMD_GET_STATUS_MESSAGES to this service, and once that happens
     * we can safely set these intervals back to minimal intervals to reduce air traffic in
     * normal use.
     */
    m_heartbeat_interval = std::chrono::seconds(1);
    m_sys_time_interval = std::chrono::seconds(1);

    Microservice::setup();
    start_udp_read();

    /*
     * Signal to the rest of the early boot system that the status service is now listening,
     * so it's safe to start sending messages and status events.
     */
    FILE *fp = fopen("/tmp/status_service", "ab+");
    fclose(fp);
}


void StatusMicroservice::start_udp_read() {
    std::cerr << "StatusMicroservice::start_udp_read()" << std::endl;

    m_udp_socket.async_receive(boost::asio::buffer(data, max_length),
                               boost::bind(&StatusMicroservice::handle_udp_read,
                                           this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
}


void StatusMicroservice::handle_udp_read(const boost::system::error_code& error,
                                         size_t bytes_transferred) {
    std::cerr << "StatusMicroservice::handle_udp_read()" << std::endl;

    if (!error) {
        if (bytes_transferred == sizeof(localmessage_t)) {
            localmessage_t local_message;

            memcpy(&local_message, data, bytes_transferred);

            std::string message = (const char*)local_message.message;
            MAV_SEVERITY severity = (MAV_SEVERITY)local_message.level;

            send_status_message(severity, message);
        }
    }
    start_udp_read();
}



/*
 * Used to send a status message with a log level to any connected devices, and also store the message locally.
 *
 * This makes it possible for the system to "replay" all messages when a new device connects to the system and
 * needs to see messages that were generated before that point.
 *
 * This is primarily done so that early boot messages are preserved and available for review at any time, but is
 * useful at all times to ensure the user knows what's going on.
 */
void StatusMicroservice::send_status_message(MAV_SEVERITY severity, std::string message) {
    std::cout << "StatusMicroservice::send_status_message: " << message << ", sysid=" << (uint16_t)this->m_sysid << std::endl;

    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    /*
     * Store the message first, so we can send all local messages to any GCS that requests them later on
     */
    StatusMessage m;
    m.message = message;
    m.severity = severity;
    m.timestamp = timestamp;
    m_status_messages.push_back(m);

    /*
     * Now send it out
     */
    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
    int len = 0;

    char text[50] = {0};
    strncpy(text, message.c_str(), sizeof(text));
    if (text[49] != '\0') {
        text[49] = '\0';
    }


    mavlink_message_t outgoing_msg;
    mavlink_msg_openhd_status_message_pack(this->m_sysid, this->m_compid, &outgoing_msg, 0, 0, severity, text, timestamp);
    len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

    this->m_socket->async_send(boost::asio::buffer(raw, len),
                               boost::bind(&Microservice::handle_write,
                                           this,
                                           boost::asio::placeholders::error));
}


void StatusMicroservice::process_mavlink_message(mavlink_message_t msg) {
        switch (msg.msgid) {
        case MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE: {

            mavlink_openhd_status_message_t status;
            mavlink_msg_openhd_status_message_decode(&msg, &status);

            break;
        }
        case MAVLINK_MSG_ID_COMMAND_LONG: {
            mavlink_command_long_t command;
            mavlink_msg_command_long_decode(&msg, &command);

            // only process commands sent to this system or broadcast to all systems
            if ((command.target_system != this->m_sysid && command.target_system != 0)) {
                return;
            }

            // only process commands sent to this component or boadcast to all components on this system
            if ((command.target_component != this->m_compid && command.target_component != MAV_COMP_ID_ALL)) {
                return;
            }

            switch (command.command) {
                case OPENHD_CMD_GET_STATUS_MESSAGES: {
                    std::cout << "OPENHD_CMD_GET_STATUS_MESSAGES" << std::endl;
                    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                    int len = 0;

                    /*
                     * Now we can safely set these intervals back to minimal intervals to reduce air traffic in
                     * normal use.
                     */
                    m_heartbeat_interval = std::chrono::seconds(5);
                    m_sys_time_interval = std::chrono::seconds(5);

                    // acknowledge the command, then reply
                    mavlink_message_t ack;
                    mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
                                                 this->m_compid,  // and from this component
                                                 &ack,
                                                 OPENHD_CMD_GET_STATUS_MESSAGES, // the command we're ack'ing
                                                 MAV_CMD_ACK_OK,
                                                 0,
                                                 0,
                                                 msg.sysid, // send ack to the senders system ID...
                                                 msg.compid); // ... and the senders component ID
                    len = mavlink_msg_to_send_buffer(raw, &ack);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));


                    for (auto & message : m_status_messages) {
                        uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                        int len = 0;

                        char text[50] = {0};
                        strncpy(text, message.message.c_str(), sizeof(text));
                        if (text[49] != '\0') {
                            text[49] = '\0';
                        }


                        mavlink_message_t outgoing_msg;
                        mavlink_msg_openhd_status_message_pack(this->m_sysid, this->m_compid, &outgoing_msg, msg.sysid, msg.compid, message.severity, text, message.timestamp);
                        len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

                        this->m_socket->async_send(boost::asio::buffer(raw, len),
                                                boost::bind(&Microservice::handle_write,
                                                            this,
                                                            boost::asio::placeholders::error));
                    }
                    break;
                }
                case OPENHD_CMD_GET_VERSION: {
                    std::cout << "OPENHD_CMD_GET_VERSION" << std::endl;
                    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                    int len = 0;

                    // acknowledge the command, then reply
                    mavlink_message_t ack;
                    mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
                                                 this->m_compid,  // and from this component
                                                 &ack,
                                                 OPENHD_CMD_GET_VERSION, // the command we're ack'ing
                                                 MAV_CMD_ACK_OK,
                                                 0,
                                                 0,
                                                 msg.sysid, // send ack to the senders system ID...
                                                 msg.compid); // ... and the senders component ID
                    len = mavlink_msg_to_send_buffer(raw, &ack);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));

                    char text[30] = {0};

                    strncpy(text, m_openhd_version.c_str(), sizeof(text));
                    if (text[29] != '\0') {
                        text[29] = '\0';
                    }

                    mavlink_message_t outgoing_msg;
                    mavlink_msg_openhd_version_message_pack(this->m_sysid, this->m_compid, &outgoing_msg, msg.sysid, msg.compid, text);
                    len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}
