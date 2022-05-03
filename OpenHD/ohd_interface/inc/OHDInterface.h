//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OPENHD_INTERFACE_H
#define OPENHD_OPENHD_INTERFACE_H

#include <memory>

#include "streams.h"
#include "wifi.h"
#include "ethernet.h"

class OHDInterface {
public:
    /**
     * Note: due to json, needs to be crated after OHDSystem has been run once.
     * The constructor reads the generated json by OHDSystem and tries to initialize all its members.
     * NOTE: We need to wire link - related settings into here.
     * For example, changing the wifi frequency (needs to be synced between air and ground) but also
     * settings that can be unsinced (like enable / disable wifi hotspot, which - as an example - is only
     * a setting that affects the ground pi.
     * @param is_air true if we run on the air pi, ground pi otherwise
     * @param unit_id ?? Stephen no idea ?
     */
    OHDInterface(boost::asio::io_service& io_service,bool is_air,std::string unit_id);
    std::unique_ptr<WiFi> wifi;
    std::unique_ptr<Ethernet> ethernet;
    std::unique_ptr<Streams> streams;
    // TODO: here we can add setters / getters for interface / link / wifi related settings.
    // For example, setting the wifi card power.
private:
    boost::asio::io_service& io_service;
    const bool is_air;
    const std::string unit_id;
};


#endif //OPENHD_OPENHD_INTERFACE_H
