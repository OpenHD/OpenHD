

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include <openhd/mavlink.h>

#include "json.hpp"
#include "inja.hpp"
using namespace inja;
using json = nlohmann::json;

#include "openhd-status.hpp"
#include "openhd-settings.hpp"

#include "../inc/powermicroservice.h"


constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER1;


PowerMicroservice::PowerMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id): Microservice(io_service, platform), m_is_air(is_air), m_unit_id(unit_id) {
    set_compid(SERVICE_COMPID);
}


void PowerMicroservice::setup() {
    std::cout << "PowerMicroservice::setup()" << std::endl;
    Microservice::setup();

    process_manifest();
    process_settings();
}


void PowerMicroservice::process_manifest() {
	// No manifest to process for now
}

void PowerMicroservice::process_settings() {
	// No settings for now
}


void PowerMicroservice::configure() {
    std::cerr << "Configuring power uService" << std::endl;
    // Nothing to do here

}


void PowerMicroservice::process_mavlink_message(mavlink_message_t msg) {
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_COMMAND_LONG: {
            mavlink_command_long_t command;
            mavlink_msg_command_long_decode(&msg, &command);

            std::cerr << "Long command received, target system=" << (uint16_t)command.target_system << "; target comp=" << (uint16_t)command.target_component << "; command=" << command.command << "\n";
            std::cerr << "My component id=" << (uint16_t) this->m_compid << "\n";

            // only process commands sent to this system or broadcast to all systems
            if ((command.target_system != this->m_sysid && command.target_system != 0)) {
                return;
            }

            // only process commands sent to this component or boadcast to all components on this system
		// add compid 200 for betaflight same fix aß here https://github.com/OpenHD/Open.HD/commit/d7e9c9956601235d105196616c20d68443a73be1
            if ((command.target_component != this->m_compid && command.target_component != MAV_COMP_ID_ALL && command.target_component != 200)) {
                return;
            }

            switch (command.command) {
            	case OPENHD_CMD_POWER_SHUTDOWN: {
            		std::cerr << "Shutdown command received!\n";
                    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                    int len = 0;
					mavlink_message_t ack;
					mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
												 this->m_compid,  // and from this component
												 &ack,
												 OPENHD_CMD_POWER_SHUTDOWN, // the command we're ack'ing
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

					sync();
					sleep(3);

            		boost::process::child c_systemctl_shutdown(
            				boost::process::search_path("systemctl"),
            				std::vector<std::string>{"start", "poweroff.target"},
							m_io_service);
            		c_systemctl_shutdown.detach();
            		break;
            	}

            	case OPENHD_CMD_POWER_REBOOT: {
            		std::cerr << "Reboot command received!\n";
            		boost::process::child c_systemctl_reboot(
            				boost::process::search_path("systemctl"),
							std::vector<std::string>{"start", "reboot.target"},
							m_io_service);
            		c_systemctl_reboot.detach();
            		uint8_t raw[MAVLINK_MAX_PACKET_LEN];
            		int len = 0;

            		// acknowledge the command, no reply
            		mavlink_message_t ack;
            		mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
            				this->m_compid,  // and from this component
							&ack,
							OPENHD_CMD_POWER_REBOOT, // the command we're ack'ing
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

					sync();
					sleep(3);
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

void PowerMicroservice::save_settings() {
    

}

