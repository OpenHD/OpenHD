#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "DPlatform.h"

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover all connected cameras and write them to json.
 */
class DCameras: public OHD::IDiscoverable{
public:
    DCameras(PlatformType platform_type, BoardType board_type, CarrierType carrier_type);
    
    virtual ~DCameras() = default;

    void discover() override;

    nlohmann::json generate_manifest() override;

    [[nodiscard]] int count() const{
        return (int)m_cameras.size();
    }
private:
    /*
     * These are for platform-specific camera access methods, most can also be accessed through v4l2
     * but there are sometimes downsides to doing that. For example on the Pi, v4l2 can have higher latency
     * than going through the broadcom API, and at preset bcm2835-v4l2 doesn't support all of the ISP controls.
     *
     */
    void detect_raspberrypi_csi();
    void detect_raspberrypi_veye();
    void detect_jetson_csi();
    void detect_rockchip_csi();

    void detect_flir();
    void detect_seek();
    
    void detect_v4l2();
    void probe_v4l2_device(std::string device_node);
    bool process_video_node(Camera& camera, CameraEndpoint& endpoint, std::string node);

    void detect_ip();

    std::vector<Camera> m_cameras;
    std::vector<CameraEndpoint> m_camera_endpoints;

    int m_discover_index = 0;

    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;
};

#endif
