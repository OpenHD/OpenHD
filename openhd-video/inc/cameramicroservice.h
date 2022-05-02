#ifndef CAMERAMICROSERVICE_H
#define CAMERAMICROSERVICE_H

#include <openhd/mavlink.h>

#include <array>
#include <stdexcept>
#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include "openhd-camera.hpp"
#include "openhd-microservice.hpp"
#include "openhd-platform.hpp"


#include "camerastream.h"


class CameraMicroservice: public Microservice {
public:
    CameraMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id);

    void setup();
    void process_manifest();
    void process_settings();
    void debug_camerastream();

    void save_settings(std::vector<Camera> cards, std::string settings_file);

    void process_mavlink_message(mavlink_message_t msg);

    void configure(Camera &camera);

private:
    //std::vector<CameraStream> m_camera_streams;
    std::vector<std::unique_ptr<CameraStream>> m_camera_streams;

    std::vector<Camera> m_cameras;

    std::string m_unit_id;

    bool m_is_air = false;

    int m_base_port = 5620;
};

#endif
