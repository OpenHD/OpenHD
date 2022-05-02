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

//#include <systemd/sd-daemon.h>

#include "openhd-platform.hpp"
#include "openhd-log.hpp"

#include "control.h"
#include "mavlinkcontrol.h"
#include "hotspot.h"
//#include "record.h"
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
    // the undocumented crap from main.cpp
    Control *control;
    Hotspot *hotspot1;
    Hotspot *hotspot2;
    Hotspot *hotspot3;
    Hotspot *hotspot4;
    CameraMicroservice * camera_microservice;
};


#endif //OPENHD_VIDEO_OHDVIDEO_H
