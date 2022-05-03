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

#include "mavlinkcontrol.h"
#include "cameramicroservice.h"
#include <string>


#include "json.hpp"

class OHDVideo {
public:
    OHDVideo(boost::asio::io_service& io_service,bool is_air,std::string unit_id,PlatformType platform_type);
private:
    boost::asio::io_service& io_service;
    const bool is_air;
    const std::string unit_id;
    const PlatformType platform_type;
private:
    // Only set when running on the air pi,
    // This will probably get un- spaghetti soon.
    // For now we need it, since it is what actually starts the streaming (I think)
    std::unique_ptr<CameraMicroservice> camera_microservice;
};


#endif //OPENHD_VIDEO_OHDVIDEO_H
