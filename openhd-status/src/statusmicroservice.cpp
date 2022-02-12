#include "../inc/statusmicroservice.h"

#include <array>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/reboot.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <boost/regex.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <openhd/mavlink.h>

#include "openhd-platform.hpp"

#include "../../openhd-common/openhd-status.hpp"


uint32_t videoBitrate=0;
uint32_t temp_air,temp_gnd,load_gnd,load_air = 0;
int32_t rssi_air = 0;

constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER3;

typedef struct {
    uint32_t received_packet_cnt;
    int8_t current_signal_dbm;
    int8_t type; // 0 = Atheros, 1 = Ralink
    int8_t signal_good;
} __attribute__((packed)) wifi_adapter_rx_status_forward_t;


typedef struct {
    uint32_t damaged_block_cnt; // number bad blocks video downstream
    uint32_t lost_packet_cnt; // lost packets video downstream
    uint32_t skipped_packet_cnt; // skipped packets video downstream
    uint32_t injection_fail_cnt;  // Video injection failed downstream
    uint32_t received_packet_cnt; // packets received video downstream
    uint32_t kbitrate; // live video kilobitrate per second video downstream
    uint32_t kbitrate_measured; // max measured kbitrate during tx startup
    uint32_t kbitrate_set; // set kilobitrate (measured * bitrate_percent) during tx startup
    uint32_t lost_packet_cnt_telemetry_up; // lost packets telemetry uplink
    uint32_t lost_packet_cnt_telemetry_down; // lost packets telemetry downlink
    uint32_t lost_packet_cnt_msp_up; // lost packets msp uplink (not used at the moment)
    uint32_t lost_packet_cnt_msp_down; // lost packets msp downlink (not used at the moment)
    uint32_t lost_packet_cnt_rc; // lost packets rc link
    int8_t current_signal_joystick_uplink; // signal strength in dbm at air pi (telemetry upstream and rc link)
    int8_t current_signal_telemetry_uplink;
    int8_t joystick_connected; // 0 = no joystick connected, 1 = joystick connected
    float HomeLat;
    float HomeLon;
    uint8_t cpuload_gnd; // CPU load Ground Pi
    uint8_t temp_gnd; // CPU temperature Ground Pi
    uint8_t cpuload_air; // CPU load Air Pi
    uint8_t temp_air; // CPU temperature Air Pi
    uint32_t wifi_adapter_cnt; // number of wifi adapters
    wifi_adapter_rx_status_forward_t adapter[6]; // same struct as in wifibroadcast lib.h
} __attribute__((packed)) wifibroadcast_rx_status_forward_t;

#define MAX_RX_INTERFACES 8;

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
        if (bytes_transferred == sizeof(localWifiStatusmessage_t)) {
            localWifiStatusmessage_t in_message;
            memcpy(&in_message, data, bytes_transferred);
                
            if (!m_is_air && (in_message.radioport == 56)) {
                wifibroadcast_rx_status_forward_t out_message;
                
                int cpuload_gnd = 0;
                int temp_gnd = 0;
                long double a[4];

                FILE *fp;
                FILE *fp2;

                try {
                    fp2 = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
                    fscanf(fp2, "%d", &temp_gnd);
                } catch (...){
                    std::cout << "ERROR: thermal reading" << std::endl;
                }
                fclose(fp2);
                temp_gnd= temp_gnd/1000;

                try {
                    fp = fopen("/proc/stat", "r");
                    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
                } catch (...){
                    std::cout << "ERROR: proc reading1" << std::endl;
                }
                fclose(fp);
                
                cpuload_gnd = (a[0] + a[1] + a[2]) / (a[0] + a[1] + a[2] + a[3]) * 100;

                uint32_t bitrate = videoBitrate; //Scale factor only to tune to real measurement
                out_message.damaged_block_cnt = (uint32_t)in_message.count_p_bad;
                out_message.lost_packet_cnt = (uint32_t)in_message.count_p_lost;
                out_message.skipped_packet_cnt = (uint32_t)in_message.count_p_bad;
                out_message.injection_fail_cnt = (uint32_t)in_message.count_p_lost;
                out_message.received_packet_cnt = (uint32_t)in_message.count_p_all;
                out_message.kbitrate = bitrate;
                out_message.kbitrate_measured = bitrate;
                out_message.kbitrate_set = bitrate;
                out_message.lost_packet_cnt_telemetry_up =0;
                out_message.lost_packet_cnt_telemetry_down =0;
                out_message.lost_packet_cnt_msp_up=0;
                out_message.lost_packet_cnt_msp_down=0;
                out_message.lost_packet_cnt_rc=0;
                out_message.current_signal_joystick_uplink=rssi_air;
                out_message.current_signal_telemetry_uplink=0;
                out_message.joystick_connected=0;
                out_message.HomeLat=0;
                out_message.HomeLon=0;
                out_message.cpuload_gnd = cpuload_gnd;
                out_message.temp_gnd = temp_gnd;
                out_message.cpuload_air = load_air;
                out_message.temp_air = temp_air;
                
                int8_t nr_of_cards=0;

                for (int8_t j = 0; (j < 6) && (in_message.rssiPerCard[j].count_all > 0); ++j) {
                    out_message.adapter[j].received_packet_cnt = in_message.rssiPerCard[j].count_all;
                    if (out_message.adapter[j].received_packet_cnt==0)
                        out_message.adapter[j].current_signal_dbm = -127;
                    else
                        out_message.adapter[j].current_signal_dbm = in_message.rssiPerCard[j].rssi_min;
                    out_message.adapter[j].type = 0;
                    out_message.adapter[j].signal_good = 0;
                    nr_of_cards++;
                }
        
                out_message.wifi_adapter_cnt = nr_of_cards;

                struct sockaddr_in si_other_rssi;
                int s_rssi, slen_rssi = sizeof(si_other_rssi);
                int broadcast = 1;
                si_other_rssi.sin_family = AF_INET;
                si_other_rssi.sin_port = htons(5155);
                si_other_rssi.sin_addr.s_addr = inet_addr("127.0.0.1");

                memset(si_other_rssi.sin_zero, '\0', sizeof(si_other_rssi.sin_zero));

                if ((s_rssi = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
                    std::cout << "ERROR: Could not create UDP socket!" << std::endl;
                } 
                else if (sendto(s_rssi, &out_message, sizeof(out_message), 0, (struct sockaddr*)&si_other_rssi, slen_rssi) == -1) {
                    std::cout << "ERROR: Could not send RSSI data to localhost!" << std::endl;
                }
                close (s_rssi);

                // -- REMOVE IN FINAL --
                // Below should be replaced by mavlink telemetry packages
                si_other_rssi.sin_port = htons(5154);
                si_other_rssi.sin_addr.s_addr = inet_addr("192.168.2.255");
                
                if ((s_rssi = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
                    std::cout << "ERROR: Could not create UDP socket!" << std::endl;
                    exit(1);
                } 

                if (setsockopt(s_rssi, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
                    std::cout << "setsockopt (SO_BROADCAST)" << std::endl;
                    exit(1);
                }

                if (sendto(s_rssi, &out_message, sizeof(out_message), 0, (struct sockaddr*)&si_other_rssi, slen_rssi) == -1) {
                    std::cout << "ERROR: Could not broadcast RSSI data!" << std::endl;
                }
                close (s_rssi);
                
                //Broadcast to all ethernet hotspot connected devices
                si_other_rssi.sin_addr.s_addr = inet_addr("192.168.3.255");
                
                if ((s_rssi = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
                    std::cout << "ERROR: Could not create UDP socket!" << std::endl;
                    exit(1);
                } 

                if (setsockopt(s_rssi, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
                    std::cout << "setsockopt (SO_BROADCAST)" << std::endl;
                    exit(1);
                }

                if (sendto(s_rssi, &out_message, sizeof(out_message), 0, (struct sockaddr*)&si_other_rssi, slen_rssi) == -1) {
                    std::cout << "ERROR: Could not broadcast RSSI data!" << std::endl;
                }
                close (s_rssi);

            } else {
                std::cerr << "Air radioport:" << +in_message.radioport << std::endl;

                if (in_message.radioport < 10)
                    Send_air_telemetrystatus(in_message, bytes_transferred);
                Send_air_load();
            }
            
            // -- END REMOVE --
        }

        boost::smatch result;
        boost::regex reg{"vbr([0-9]*)end"};
        std::string input(data, bytes_transferred);

        if (boost::regex_search(input, result, reg)){
            //std::cerr << "regex: _" << input << "_ " << result.size() << " :" << result[1] << std::endl;
            if (result.size()==2) {
                std::cerr << "Video " << result[1] << "kbit" << std::endl;
                videoBitrate = std::stoi(result[1]);
            }
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

//void StatusMicroservice::Send_air_load(const boost::system::error_code& error) {
//void Send_air_telemetrystatus(localWifiStatusmessage_t msg,
//                                size_t bytes_transferred);
void StatusMicroservice::Send_air_telemetrystatus(localWifiStatusmessage_t& msg, size_t bytes_transferred) {
    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
    int len = 0;

    mavlink_message_t outgoing_msg;
    memcpy(&msg, data, bytes_transferred);
    static int32_t rssi = 0;
    static int rssi_timeout = 0;

    if (msg.rssiPerCard[0].count_all) {
        rssi = (msg.rssiPerCard[0].rssi_sum)/(msg.rssiPerCard[0].count_all);
        rssi_timeout=3; // Three concurrent messages with no RSSI before celaring it
    } else {
        if ((rssi_timeout--)<0) {
            rssi = 0;
            rssi_timeout = 0;
        }
    }
    std::cerr << "Air RSSI " << +rssi << std::endl;
    mavlink_msg_openhd_air_telemetry_pack(this->m_sysid, this->m_compid, &outgoing_msg, 254, this->m_compid, msg.count_p_bad, msg.count_p_lost, 0, 0, rssi);
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

}


void StatusMicroservice::Send_air_load() {
    std::cout << "Microservice::status::send_air_load" << std::endl;

    int cpuload = 0;
    int temp = 0;
    long double a[4], b[4];

    FILE *fp;
    FILE *fp2;

    try {
        fp2 = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        fscanf(fp2, "%d", &temp);
    } catch (...){
        std::cout << "ERROR: thermal reading" << std::endl;
    }
    fclose(fp2);
    temp = temp /1000;

    try {
        fp = fopen("/proc/stat", "r");
        fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    } catch (...){
        std::cout << "ERROR: proc reading1" << std::endl;
    }
    fclose(fp);
    long tmp = ((a[0] + a[1] + a[2]) / (a[0] + a[1] + a[2] + a[3])) * 100;
    cpuload = tmp; 

    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
    int len = 0;

    mavlink_message_t outgoing_msg;

    /**
    * @brief Pack a openhd_air_load message
    * @param system_id ID of this system
    * @param component_id ID of this component (e.g. 200 for IMU)
    * @param msg The MAVLink message to compress the data into
    *
    * @param target_system  system id of the requesting system
    * @param target_component  component id of the requesting component
    * @param cpuload  cpuload
    * @param temp  temp
    * @return length of the message in bytes (excluding serial stream start sign)
    */
    //static inline uint16_t mavlink_msg_openhd_air_load_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
    //                            uint8_t target_system, uint8_t target_component, uint8_t cpuload, uint8_t temp)


    len = mavlink_msg_openhd_air_load_pack(this->m_sysid, this->m_compid, &outgoing_msg, 0, this->m_compid, cpuload, temp);
    //mavlink_msg_heartbeat_pack(this->m_sysid, this->m_compid, &outgoing_msg, MAV_TYPE_GENERIC, MAV_AUTOPILOT_INVALID, 0, 0, 0);
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
}

void StatusMicroservice::process_mavlink_message(mavlink_message_t msg) {
        switch (msg.msgid) {
        case MAVLINK_MSG_ID_OPENHD_AIR_LOAD: {
            mavlink_openhd_air_load_t air_load_msg;
            mavlink_msg_openhd_air_load_decode(&msg, &air_load_msg);
            load_air = air_load_msg.cpuload;
            temp_air = air_load_msg.temp;
            std::cerr << "air temp " << temp_air << " cpu load " << load_air << std::endl;      
        }

        case MAVLINK_MSG_ID_OPENHD_AIR_TELEMETRY: {
            //mavlink_openhd_air_telemetry_t status_msg;
            //mavlink_msg_openhd_status_message_decode(&msg, &status_msg);
            rssi_air = (int8_t) mavlink_msg_openhd_air_telemetry_get_current_signal_dbm(&msg);
            std::cerr << "air RSSI " << +rssi_air << std::endl;
        }
        
        case MAVLINK_MSG_ID_OPENHD_STATUS_MESSAGE: {

            mavlink_openhd_status_message_t status;
            mavlink_msg_openhd_status_message_decode(&msg, &status);

            break;
        }
        case MAVLINK_MSG_ID_COMMAND_LONG: {
            mavlink_command_long_t command;
            mavlink_msg_command_long_decode(&msg, &command);

            if ((command.target_system != this->m_sysid && command.target_system != 0)) {
                std::cerr << "message received:" << +command.target_system << " " << command.target_component << std::endl;
                mavlink_command_long_t command;
                mavlink_msg_command_long_decode(&msg, &command);
                std::cerr << "decoded long message"<< std::endl;
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
