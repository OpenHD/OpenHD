

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "camerastream.h"



CameraStream::CameraStream(boost::asio::io_service &io_service, 
                           PlatformType platform,
                           Camera &camera, 
                           uint16_t port): 
    m_io_service(io_service),
    m_platform_type(platform),
    m_camera(camera),
    m_video_port(port) {}


void CameraStream::setup() {}
void CameraStream::start() {}
void CameraStream::stop() {}
bool CameraStream::supports_bitrate() { return false; }
void CameraStream::set_bitrate(int bitrate) {}
bool CameraStream::supports_cbr() { return false; }
void CameraStream::set_cbr(bool enable) {}
std::vector<std::string> CameraStream::get_supported_formats() {}
std::string CameraStream::get_format() {}
void CameraStream::set_format(std::string format) {}

