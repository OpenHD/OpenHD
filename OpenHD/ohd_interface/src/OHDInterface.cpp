//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

OHDInterface::OHDInterface(boost::asio::io_service& io_service,bool is_air,std::string unit_id):io_service(io_service),is_air(is_air),unit_id(unit_id){
    wifi=std::make_unique<WiFi>(io_service, is_air, unit_id);
    ethernet=std::make_unique<Ethernet>(io_service, is_air, unit_id);
    streams=std::make_unique<WBStreams>(is_air, unit_id);
    try {
        wifi->configure();
        ethernet->configure();
        streams->set_broadcast_cards(wifi->broadcast_cards());
        streams->configure();
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }
}