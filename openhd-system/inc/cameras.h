#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "platform.h"


typedef enum CameraType {
    CameraTypeRaspberryPiCSI,
    CameraTypeJetsonCSI,
    CameraTypeRockchipCSI,
    CameraTypeUVC,
    CameraTypeIP,
    CameraTypeUnknown
} CameraType;


struct CameraEndpoint {
    std::string device_node;
    std::string bus;
    bool support_h264 = false;
    bool support_h265 = false;
    bool support_mjpeg = false;
    bool support_raw = false;
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



class Cameras {
public:
    Cameras(PlatformType platform_type, BoardType board_type, CarrierType carrier_type);
    
    virtual ~Cameras() {}

    void discover();

    std::string generate_manifest();

private:
    /*
     * These are for platform-specific camera access methods, most can also be accessed through v4l2
     * but there are sometimes downsides to doing that. For example on the Pi, v4l2 can have higher latency
     * than going through the broadcom API, and at preset bcm2835-v4l2 doesn't support all of the ISP controls.
     *
     */
    void detect_raspberrypi_csi();
    void detect_jetson_csi();
    void detect_rockchip_csi();


    void detect_v4l2();
    void probe_v4l2_device(std::string device_node);
    bool process_video_node(Camera& camera, CameraEndpoint& endpoint, std::string node);

    void detect_ip();
    void probe_ip_camera(std::string url);

    std::string camera_type_string(CameraType camera_type);

    std::vector<Camera> m_cameras;
    std::vector<CameraEndpoint> m_camera_endpoints;

    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;
};

#endif
