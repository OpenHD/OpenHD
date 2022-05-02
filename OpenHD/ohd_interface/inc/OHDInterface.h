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
     * @param is_air
     * @param unit_id
     */
    OHDInterface(boost::asio::io_service& io_service,bool is_air,std::string unit_id);
    std::unique_ptr<WiFi> wifi;
    std::unique_ptr<Ethernet> ethernet;
    std::unique_ptr<Streams> streams;
private:
    boost::asio::io_service& io_service;
    const bool is_air;
    const std::string unit_id;
};


#endif //OPENHD_OPENHD_INTERFACE_H
