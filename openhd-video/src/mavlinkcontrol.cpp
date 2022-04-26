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

#include <json.hpp>

#include <openhd/mavlink.h>

#include "openhd-platform.hpp"
#include "openhd-status.hpp"

#include "mavlinkcontrol.h"

constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER6;


MavlinkControl::MavlinkControl(boost::asio::io_service &io_service, PlatformType platform): Microservice(io_service, platform) {
    set_compid(SERVICE_COMPID);
}


void MavlinkControl::setup() {
    std::cout << "MavlinkControl::setup()" << std::endl;
    process_manifest();

    Microservice::setup();
}


void MavlinkControl::process_manifest() {
    try {
        std::ifstream f("/tmp/profile_manifest");
        nlohmann::json j;
        f >> j;

        m_is_air = j["is-air"];
        m_sysid = j["microservice-sys-id"];

    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        ohd_log(STATUS_LEVEL_EMERGENCY, "Profile manifest processing failed");
        std::cerr << "MavlinkControl::process_manifest: " << ex.what() << std::endl;
        return;
    }
}


void MavlinkControl::command_done() {

}


void MavlinkControl::command_failed() {
    ohd_log(STATUS_LEVEL_ALERT, "Video control command failed");
}


void MavlinkControl::process_mavlink_message(mavlink_message_t msg) {
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_HEARTBEAT: {
            mavlink_heartbeat_t heartbeat;
            mavlink_msg_heartbeat_decode(&msg, &heartbeat);
            //MAV_STATE state = (MAV_STATE)heartbeat.system_status;
            MAV_MODE_FLAG mode = (MAV_MODE_FLAG)heartbeat.base_mode;

            auto autopilot = (MAV_AUTOPILOT)heartbeat.autopilot;

            if (autopilot != MAV_AUTOPILOT_INVALID) {
                if (mode & MAV_MODE_FLAG_SAFETY_ARMED) {
                    armed(true);
                } else {
                    armed(false);
                }
            }
            break;
        }
        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: {
            mavlink_param_request_list_t request;
            mavlink_msg_param_request_list_decode(&msg, &request);
            break;
        }
        case MAVLINK_MSG_ID_RC_CHANNELS:{
            mavlink_rc_channels_t _rc_channels;
            mavlink_msg_rc_channels_decode(&msg, &_rc_channels);

            std::vector<uint16_t> _channels = {
                _rc_channels.chan1_raw,
                _rc_channels.chan2_raw,
                _rc_channels.chan3_raw,
                _rc_channels.chan4_raw,
                _rc_channels.chan5_raw,
                _rc_channels.chan6_raw,
                _rc_channels.chan7_raw,
                _rc_channels.chan8_raw,
                _rc_channels.chan9_raw,
                _rc_channels.chan10_raw,
                _rc_channels.chan11_raw,
                _rc_channels.chan12_raw,
                _rc_channels.chan13_raw,
                _rc_channels.chan14_raw,
                _rc_channels.chan15_raw,
                _rc_channels.chan16_raw,
                _rc_channels.chan17_raw,
                _rc_channels.chan18_raw
            };

            rc_channels(_channels);

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
            // add compid 200 for betaflight same fix aß here https://github.com/OpenHD/Open.HD/commit/d7e9c9956601235d105196616c20d68443a73be1
            if ((command.target_component != this->m_compid && command.target_component != MAV_COMP_ID_ALL && command.target_component != 200)) {
                return;
            }

            // there aren't any commands yet
            switch (command.command) {
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

