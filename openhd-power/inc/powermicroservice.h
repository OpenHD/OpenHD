#ifndef POWERMICROSERVICE_H
#define POWERMICROSERVICE_H

#include <openhd/mavlink.h>

#include <array>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-microservice.hpp"
#include "openhd-platform.hpp"




class PowerMicroservice: public Microservice {
public:
    PowerMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id);

    void setup();
    void process_manifest();
    void process_settings();

    void save_settings();

    void process_mavlink_message(mavlink_message_t msg);

    void configure();

private:

    std::string m_unit_id;

    bool m_is_air = false;

    int m_base_port = 5620;
};

#endif
