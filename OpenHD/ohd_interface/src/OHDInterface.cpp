//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

OHDInterface::OHDInterface(bool is_air,std::string unit_id):is_air(is_air),unit_id(unit_id){
    std::cout<<"OHDInterface::OHDInterface()\n";
    wifi=std::make_unique<WiFi>(is_air, unit_id);
    ethernet=std::make_unique<Ethernet>(is_air, unit_id);
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
    std::cout<<"OHDInterface::created\n";
}