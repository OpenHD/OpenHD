#ifndef OPENHD_UTIL_H
#define OPENHD_UTIL_H



#include "openhd-types.h"
#include "openhd-structs.h"


inline std::string camera_type_to_string(CameraType camera_type) {
    switch (camera_type) {
        case CameraTypeRaspberryPiCSI: {
            return "pi-csi";
        }
        case CameraTypeJetsonCSI: {
            return "jetson-csi";
        }
        case CameraTypeRockchipCSI: {
            return "rockchip-csi";
        }
        case CameraTypeUVC: {
            return "uvc";
        }
        case CameraTypeIP: {
            return "ip";
        }
        default: {
            return "unknown";
        }
    }
}


#endif