#ifndef OPENHD_ETHERNET_H
#define OPENHD_ETHERNET_H


#include <string>

#include "openhd-util.hpp"


typedef enum EthernetCardType {
    EthernetCardTypeiPhoneTether,
    EthernetCardTypeAndroidTether,
    EthernetCardTypeUnknown
} EthernetCardType;


typedef enum EthernetHotspotType {
    EthernetHotspotTypeInternal,
    EthernetHotspotTypeNone
} EthernetHotspotType;



struct EthernetCard {
    EthernetCardType type;
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string mac;
    std::string vid;
    std::string pid;
    std::string usb_bus;
};


inline std::string ethernet_card_type_to_string(EthernetCardType card_type) {
    switch (card_type) {
        case EthernetCardTypeiPhoneTether: {
            return "ipheth";
        }
        case EthernetCardTypeAndroidTether: {
            return "rndis_host";
        }
        default: {
            return "unknown";
        }
    }
}


inline EthernetCardType string_to_ethernet_card_type(std::string driver_name) {
    if (to_uppercase(driver_name).find(to_uppercase("ipheth")) != std::string::npos) {
        return EthernetCardTypeiPhoneTether;
    } else if (to_uppercase(driver_name).find(to_uppercase("rndis_host")) != std::string::npos) {
        return EthernetCardTypeAndroidTether;
    }

    return EthernetCardTypeUnknown;
}


inline std::string ethernet_hotspot_type_to_string(EthernetHotspotType ethernet_hotspot_type) {
    switch (ethernet_hotspot_type) {
        case EthernetHotspotTypeInternal: {
            return "internal";
        }
        default: {
            return "none";
        }
    }
}


inline EthernetHotspotType string_to_ethernet_hotspot_type(std::string hotspot_type) {
    if (to_uppercase(hotspot_type).find(to_uppercase("internal")) != std::string::npos) {
        return EthernetHotspotTypeInternal;
    }

    return EthernetHotspotTypeNone;
}


#endif
