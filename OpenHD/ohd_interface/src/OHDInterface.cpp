//
// Created by consti10 on 02.05.22.
//

#include "OHDInterface.h"

OHDInterface::OHDInterface(const OHDProfile& profile):profile(profile){
    std::cout<<"OHDInterface::OHDInterface()\n";
    wifi=std::make_unique<WifiCards>(profile);
    //ethernet=std::make_unique<EthernetCards>(is_air, unit_id);
    streams=std::make_unique<WBStreams>(profile);
    try {
        wifi->configure();
        //ethernet->configure();
        streams->set_broadcast_card_names(wifi->get_broadcast_card_names());
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

void OHDInterface::debug() const {
    std::cout<<"OHDInterface::debug:begin\n";
    if(wifi){
        wifi->debug();
    }
    if(streams){
        streams->debug();
    }
    //if(ethernet){
    //    ethernet->debug();
    //}
    std::cout<<"OHDInterface::debug:end\n";
}
