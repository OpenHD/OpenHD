#ifndef OPENHD_LTE_H
#define OPENHD_LTE_H


#include <string>

#include "openhd-util.hpp"

// note: this is mostly a duplicate of EthernetCard, but is likely to have other properties and shouldn't be shared
struct LTECard {
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    std::string usb_bus;
};


#endif
