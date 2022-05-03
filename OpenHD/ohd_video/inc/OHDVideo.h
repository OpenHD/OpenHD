//
// Created by consti10 on 03.05.22.
//

#ifndef OPENHD_VIDEO_OHDVIDEO_H
#define OPENHD_VIDEO_OHDVIDEO_H

#include <fstream>

#include <iostream>
#include <iterator>
#include <exception>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "openhd-platform.hpp"
#include "openhd-log.hpp"

#include "cameramicroservice.h"
#include <string>


#include "json.hpp"

/**
 * Main entry point for OpenHD video streaming.
 * NOTE: This module only needs to be run on air pi, so to say it is a "Video stream camera wrapper".
 * TODO: We need to feed mavlink commands coming from telemetry into here & send them to the according streams
 */
class OHDVideo {
public:
    OHDVideo(boost::asio::io_service& io_service,bool is_air,std::string unit_id,PlatformType platform_type);
private:
    boost::asio::io_service& m_io_service;
    const bool m_is_air;
    const std::string m_unit_id;
    const PlatformType m_platform_type;
private:
    // Only set when running on the air pi,
    // This will probably get un- spaghetti soon.
    // For now we need it, since it is what actually starts the streaming (I think)
    std::unique_ptr<CameraMicroservice> camera_microservice;
private:
    // These members are what used to be in camera microservice
    // All the created camera streams
    std::vector<std::unique_ptr<CameraStream>> m_camera_streams;
    // each camera stream has a camera, is this duplicated ??!
    std::vector<Camera> m_cameras;
    // these methods are from camera microservice
    void setup();
    void process_manifest();
    void process_settings();
    void configure(Camera &camera);
    void save_settings(std::vector<Camera> cards, std::string settings_file);
};


#endif //OPENHD_VIDEO_OHDVIDEO_H
