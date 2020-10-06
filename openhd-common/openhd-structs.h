#ifndef OPENHD_STRUCTS_H
#define OPENHD_STRUCTS_H

#include "openhd-types.h"

#include <vector>

struct CameraEndpoint {
    std::string device_node;
    std::string bus;
    bool support_h264 = false;
    bool support_h265 = false;
    bool support_mjpeg = false;
    bool support_raw = false;

    std::vector<std::string> formats;
};


struct Camera {
    CameraType type;
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    // for USB this is the bus number, for CSI it's the connector number
    std::string bus;
};


struct WiFiCard {
    WiFiCardType type;
    std::string name;
    bool supports_5ghz;
    bool supports_2ghz;
    bool supports_injection;
    bool supports_rts;
};

struct EthernetCard {
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    std::string usb_bus;
};

// note: this is mostly a duplicate of EthernetCard, but is likely to have other properties and shouldn't be shared
struct LTECard {
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    std::string usb_bus;
};

#endif
