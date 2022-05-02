#ifndef MAVLINKCONTROL_H
#define MAVLINKCONTROL_H

#include <array>
#include <chrono>
#include <vector>
#include <stdexcept>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "json.hpp"

#include <openhd/mavlink.h>


#include "openhd-microservice.hpp"
#include "openhd-platform.hpp"

class MavlinkControl: public Microservice {
public:
    MavlinkControl(boost::asio::io_service &io_service, PlatformType platform);
    
    void setup();

    void process_mavlink_message(mavlink_message_t msg);

    void process_manifest();

    void command_done();
    void command_failed();

    boost::signals2::signal<void (bool)> armed;
    boost::signals2::signal<void (std::vector<uint16_t>)> rc_channels;

private:
    bool m_is_air = false;

    // this gets retrieved from the profile manifest when this service runs
    int m_sysid = 255;
};


#endif

