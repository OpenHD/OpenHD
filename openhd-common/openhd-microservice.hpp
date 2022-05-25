#ifndef OPENHD_MICROSERVICE_H
#define OPENHD_MICROSERVICE_H

#define MICROSERVICE_ROUTER_ADDRESS "127.0.0.1"
#define MICROSERVICE_ROUTER_PORT 5761


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <openhd/mavlink.h>

#include <array>
#include <chrono>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "openhd-platform.hpp"

typedef enum MavlinkCommandState {
    MavlinkCommandStateReady,
    MavlinkCommandStateSend,
    MavlinkCommandStateWaitACK,
    MavlinkCommandStateDone,
    MavlinkCommandStateFailed
} MavlinkCommandState;


typedef enum MavlinkCommandType {
    MavlinkCommandTypeLong,
    MavlinkCommandTypeInt
} MavlinkCommandType;



class MavlinkCommand  {
public:
    MavlinkCommand(MavlinkCommandType command_type) : m_command_type(command_type) {}
    MavlinkCommandType m_command_type;
    uint16_t command_id = 0;
    uint8_t retry_count = 0;

    uint8_t target_sysid = 255;
    uint8_t target_compid = 0;

    uint8_t long_confirmation = 0;
    float long_param1 = 0;
    float long_param2 = 0;
    float long_param3 = 0;
    float long_param4 = 0;
    float long_param5 = 0;
    float long_param6 = 0;
    float long_param7 = 0;


    uint8_t int_frame = 0;
    uint8_t int_current = 0;
    uint8_t int_autocontinue = 0;
    float int_param1 = 0;
    float int_param2 = 0;
    float int_param3 = 0;
    float int_param4 = 0;
    int   int_param5 = 0;
    int   int_param6 = 0;
    float int_param7 = 0;
};



class Microservice {
public:
    Microservice(boost::asio::io_service &io_service, PlatformType platform): m_platform_type(platform),
                                                                              m_io_service(io_service),
                                                                              m_socket(new boost::asio::ip::tcp::socket(io_service)), 
                                                                              m_boot_time(std::chrono::steady_clock::now()),
                                                                              m_heartbeat_interval(5), 
                                                                              m_heartbeat_timer(io_service, m_heartbeat_interval), 
                                                                              m_sys_time_interval(5), 
                                                                              m_sys_time_timer(io_service, m_sys_time_interval),
                                                                              m_reconnect_interval(1), 
                                                                              m_reconnect_timer(io_service, m_reconnect_interval),
                                                                              m_command_interval(200), 
                                                                              m_command_timer(io_service, m_command_interval) {}


    virtual ~Microservice() {}


    void connect() {
        if (m_connected) {
            return;
        }

        m_socket.reset(new boost::asio::ip::tcp::socket(m_io_service));

        boost::system::error_code error;

        m_socket->open(boost::asio::ip::tcp::v4());
        m_socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(MICROSERVICE_ROUTER_ADDRESS), MICROSERVICE_ROUTER_PORT), error);

        if (error) {
            std::cerr << "Connection failed: " << error.message() << std::endl;
            m_socket->close();
        } else {
            m_connected = true;
            start_receive();
        }
    }


    void reconnect(const boost::system::error_code& error) {
        connect();
        
        m_reconnect_timer.expires_at(m_reconnect_timer.expires_at() + m_reconnect_interval);
        m_reconnect_timer.async_wait(boost::bind(&Microservice::reconnect, 
                                                 this, 
                                                 boost::asio::placeholders::error));
    }


    void start_receive() {
        this->m_socket->async_receive(boost::asio::buffer(this->m_recv_buf, sizeof(this->m_recv_buf)), 
                                      boost::bind(&Microservice::handle_receive, 
                                                  this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
    }


    void send_heartbeat(const boost::system::error_code& error) {
        std::cout << "Microservice::send_heartbeat" << std::endl;
        uint8_t raw[MAVLINK_MAX_PACKET_LEN];
        int len = 0;

        mavlink_message_t outgoing_msg;
        mavlink_msg_heartbeat_pack(this->m_sysid, this->m_compid, &outgoing_msg, MAV_TYPE_GENERIC, MAV_AUTOPILOT_INVALID, 0, 0, 0);
        len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

        try {
        this->m_socket->async_send(boost::asio::buffer(raw, len),
                                    boost::bind(&Microservice::handle_write,
                                                this,
                                                boost::asio::placeholders::error));
        }
        catch (...){
            std::cerr << "could not send micro service heartbeat" << std::endl;
        }
        m_heartbeat_timer.expires_at(m_heartbeat_timer.expires_at() + m_heartbeat_interval);
        this->m_heartbeat_timer.async_wait(boost::bind(&Microservice::send_heartbeat, 
                                                       this, 
                                                       boost::asio::placeholders::error));
    }


    void send_system_time(const boost::system::error_code& error) {
        std::cout << "Microservice::send_system_time" << std::endl;
        uint8_t raw[MAVLINK_MAX_PACKET_LEN];
        int len = 0;

        uint64_t boot_time = std::chrono::duration_cast<std::chrono::milliseconds>(m_boot_time.time_since_epoch()).count();

        uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

        mavlink_message_t outgoing_msg;
        mavlink_msg_system_time_pack(this->m_sysid, this->m_compid, &outgoing_msg, now, boot_time);
        len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

        try {
            this->m_socket->async_send(boost::asio::buffer(raw, len),
                                    boost::bind(&Microservice::handle_write,
                                                this,
                                                boost::asio::placeholders::error));
        }
        catch (...) {
            std::cerr << "Error: could not send Microservice system time" << std::endl;
        }
        m_sys_time_timer.expires_at(m_sys_time_timer.expires_at() + m_sys_time_interval);
        this->m_sys_time_timer.async_wait(boost::bind(&Microservice::send_system_time, 
                                                      this, 
                                                      boost::asio::placeholders::error));
    }


    void set_sysid(uint8_t sysid) {
        this->m_sysid = sysid;
    }


    void set_compid(uint8_t compid) {
        this->m_compid = compid;
    }


    virtual void setup() {
        std::cout << "Microservice::setup()" << std::endl;
        this->m_heartbeat_timer.async_wait(boost::bind(&Microservice::send_heartbeat, 
                                                       this, 
                                                       boost::asio::placeholders::error));

        this->m_reconnect_timer.async_wait(boost::bind(&Microservice::reconnect, 
                                                       this, 
                                                       boost::asio::placeholders::error));

        this->m_sys_time_timer.async_wait(boost::bind(&Microservice::send_system_time, 
                                                      this, 
                                                      boost::asio::placeholders::error));

        this->m_command_timer.async_wait(boost::bind(&Microservice::command_state_update, this));
    }


    void handle_receive(const boost::system::error_code& error, std::size_t recvlen) {
        if (error) {
            m_connected = false;
            return;
        } 
        
        mavlink_message_t msg;
        for (int i = 0; i < recvlen; i++) {
            uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)m_recv_buf[i], &msg, &m_mavlink_status);
            if (res) {
                // process ack messages in the base class, subclasses will receive a signal
                // to indicate success or failure
                if (m_current_command != nullptr && msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                    mavlink_command_ack_t ack;
                    mavlink_msg_command_ack_decode(&msg, &ack);

                    if (ack.command          != m_current_command->command_id || 
                        ack.target_system    != m_current_command->target_sysid || 
                        ack.target_component != m_current_command->target_compid) {
                        // not an ack of the current command, ignore
                        return;
                    }

                    switch (ack.result) {
                        case MAV_CMD_ACK_OK: {
                            m_command_state = MavlinkCommandStateDone;
                            break;
                        }
                        default: {
                            m_command_state = MavlinkCommandStateFailed;
                            break;
                        }
                    }
                } else {
                    process_mavlink_message(msg);
                }
            }
        }

        start_receive();
    }


    void handle_write(const boost::system::error_code &error) {
        if (error) {
            m_connected = false;
        } 
    }


    virtual void process_mavlink_message(mavlink_message_t msg) {};


    void send_command(MavlinkCommand command) {
        m_current_command.reset(new MavlinkCommand(command));
        m_command_state = MavlinkCommandStateSend;
    }


    void command_state_update() {
        switch (m_command_state) {
            case MavlinkCommandStateReady: {
                // do nothing, no command being sent
                break;
            }
            case MavlinkCommandStateSend: {
                mavlink_message_t msg;

                m_command_sent_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

                if (m_current_command->m_command_type == MavlinkCommandTypeLong) {
                    mavlink_msg_command_long_pack(m_sysid, m_compid, &msg, m_current_command->target_sysid, m_current_command->target_compid, m_current_command->command_id, m_current_command->long_confirmation, m_current_command->long_param1, m_current_command->long_param2, m_current_command->long_param3, m_current_command->long_param4, m_current_command->long_param5, m_current_command->long_param6, m_current_command->long_param7);
                } else {
                    mavlink_msg_command_int_pack(m_sysid, m_compid, &msg, m_current_command->target_sysid, m_current_command->target_compid, m_current_command->int_frame, m_current_command->command_id, m_current_command->int_current, m_current_command->int_autocontinue, m_current_command->int_param1, m_current_command->int_param2, m_current_command->int_param3, m_current_command->int_param4, m_current_command->int_param5, m_current_command->int_param6, m_current_command->int_param7);
                }
                uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
                int len = mavlink_msg_to_send_buffer(buffer, &msg);

                this->m_socket->async_send(boost::asio::buffer(buffer, len),
                                           boost::bind(&Microservice::handle_write,
                                                       this,
                                                       boost::asio::placeholders::error));

                // now wait for ack
                m_command_state = MavlinkCommandStateWaitACK;

                break;
            }
            case MavlinkCommandStateWaitACK: {
                auto current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                auto elapsed = current_timestamp - m_command_sent_timestamp;

                if (elapsed > 200) {
                    // no ack in 200ms, cancel or resend
                    if (m_current_command->retry_count >= 5) {
                        m_command_state = MavlinkCommandStateFailed;
                        m_current_command.reset();
                        return;
                    }
                    m_current_command->retry_count = m_current_command->retry_count + 1;
                    if (m_current_command->m_command_type == MavlinkCommandTypeLong) {
                        /* incremement the confirmation parameter according to the Mavlink command documentation */
                        m_current_command->long_confirmation = m_current_command->long_confirmation + 1;
                    }
                    m_command_state = MavlinkCommandStateSend;
                }
                break;
            }
            case MavlinkCommandStateDone: {
                m_current_command.reset();
                command_done();
                m_command_state = MavlinkCommandStateReady;
                break;
            }
            case MavlinkCommandStateFailed: {
                m_current_command.reset();
                command_failed();
                m_command_state = MavlinkCommandStateReady;
                break;
            }
        }

        m_command_timer.expires_at(m_command_timer.expires_at() + m_command_interval);
        this->m_command_timer.async_wait(boost::bind(&Microservice::command_state_update, this));
    }


protected:
    PlatformType m_platform_type;

    virtual void command_done() {};
    virtual void command_failed() {};

    uint64_t m_last_boot = 0;
    MavlinkCommandState m_command_state = MavlinkCommandStateReady;
    uint64_t m_command_sent_timestamp = 0;
    std::shared_ptr<MavlinkCommand> m_current_command;

    uint8_t m_sysid;
    uint8_t m_compid;
    char m_recv_buf[1024];

    bool m_connected = false;

    boost::asio::io_service &m_io_service;

    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    

    mavlink_status_t m_mavlink_status;


    std::chrono::steady_clock::time_point m_boot_time;


    std::chrono::seconds m_heartbeat_interval;
    boost::asio::steady_timer m_heartbeat_timer;

    std::chrono::seconds m_sys_time_interval;
    boost::asio::steady_timer m_sys_time_timer;

    std::chrono::seconds m_reconnect_interval;
    boost::asio::steady_timer m_reconnect_timer;

    std::chrono::milliseconds m_command_interval;
    boost::asio::steady_timer m_command_timer;
};


#endif
