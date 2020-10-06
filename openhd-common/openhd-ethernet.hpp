#ifndef OPENHD_STRUCTS_H
#define OPENHD_STRUCTS_H


#include <string>

#include "openhd-util.hpp"

struct EthernetCard {
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    std::string usb_bus;
};


#endif
