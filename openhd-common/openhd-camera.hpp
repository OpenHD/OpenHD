#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H


#include <string>
#include <vector>

#include "openhd-util.hpp"

typedef enum CameraType {
    CameraTypeRaspberryPiCSI,
    CameraTypeJetsonCSI,
    CameraTypeRockchipCSI,
    CameraTypeUVC,
    CameraTypeIP,
    CameraTypeV4L2Loopback,
    CameraTypeUnknown
} CameraType;


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
        case CameraTypeV4L2Loopback: {
            return "v4l2loopback";
        }
        default: {
            return "unknown";
        }
    }
}


inline CameraType string_to_camera_type(std::string camera_type) {
    if (to_uppercase(camera_type).find(to_uppercase("pi-csi")) != std::string::npos) {
        return CameraTypeRaspberryPiCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("jetson-csi")) != std::string::npos) {
        return CameraTypeJetsonCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("rockchip-csi")) != std::string::npos) {
        return CameraTypeRockchipCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("uvc")) != std::string::npos) {
        return CameraTypeUVC;
    } else if (to_uppercase(camera_type).find(to_uppercase("ip")) != std::string::npos) {
        return CameraTypeIP;
    } else if (to_uppercase(camera_type).find(to_uppercase("v4l2loopback")) != std::string::npos) {
        return CameraTypeV4L2Loopback;
    }

    return CameraTypeUnknown;
}

#endif
