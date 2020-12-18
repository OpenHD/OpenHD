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
    // this is not just a UVC camera that happens to support h264, it's the standard UVC H264 that only a few cameras
    // support, like the older models of the Logitech C920. Other UVC cameras may support h264, but they do it in a 
    // completely different way so we keep them separate
    CameraTypeUVCH264,
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

typedef enum VideoCodec {
    VideoCodecH264,
    VideoCodecH265,
    VideoCodecMJPEG,
    VideoCodecUnknown
} VideoCodec;


struct Camera {
    CameraType type;
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    // for USB this is the bus number, for CSI it's the connector number
    std::string bus;
    int index;


    // these come from the settings system

    // this comes from the camera itself, includes width/height/fps, we will need to automate this in QOpenHD
    std::string format;

    std::string rc_channel_record = "0";

    std::string bitrate;

    std::string brightness;
    std::string contrast;
    std::string sharpness;
    std::string rotate;

    std::string wdr;
    std::string denoise;

    std::string thermal_palette;
    std::string thermal_span;

    std::string url;

    std::string manual_pipeline;

    VideoCodec codec = VideoCodecH264;

    std::vector<CameraEndpoint> endpoints;
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
        case CameraTypeUVCH264: {
            return "uvch264";
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
    } else if (to_uppercase(camera_type).find(to_uppercase("uvch264")) != std::string::npos) {
        return CameraTypeUVCH264;
    } else if (to_uppercase(camera_type).find(to_uppercase("ip")) != std::string::npos) {
        return CameraTypeIP;
    } else if (to_uppercase(camera_type).find(to_uppercase("v4l2loopback")) != std::string::npos) {
        return CameraTypeV4L2Loopback;
    }

    return CameraTypeUnknown;
}


inline std::string video_codec_to_string(VideoCodec codec) {
    switch (codec) {
        case VideoCodecH264: {
            return "h264";
        }
        case VideoCodecH265: {
            return "h265";
        }
        case VideoCodecMJPEG: {
            return "mjpeg";
        }
        default: {
            return "unknown";
        }
    }
}


inline VideoCodec string_to_video_codec(std::string codec) {
    if (to_uppercase(codec).find(to_uppercase("h264")) != std::string::npos) {
        return VideoCodecH264;
    } else if (to_uppercase(codec).find(to_uppercase("h265")) != std::string::npos) {
        return VideoCodecH265;
    } else if (to_uppercase(codec).find(to_uppercase("mjpeg")) != std::string::npos) {
        return VideoCodecMJPEG;
    }

    return VideoCodecUnknown;
}


#endif
