

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "flironestream.h"



FlirOneStream::FlirOneStream(PlatformType platform,
                             Camera camera, 
                             uint16_t port)
    : CameraStream(platform, camera, port) {
}


void FlirOneStream::setup() {
    std::cerr << "FlirOneStream::setup()" << std::endl;

    std::string device_node;

    // todo: this needs to always use the thermal view, not just the first raw stream
    for (auto &endpoint : m_camera.endpoints) {
        if (endpoint.support_raw) {
            device_node = endpoint.device_node;
            break;
        }
    }
}


void FlirOneStream::start() {
    std::cerr << "FlirOneStream::start()" << std::endl;
}


void FlirOneStream::stop() {
    std::cerr << "FlirOneStream::stop()" << std::endl;
}


bool FlirOneStream::supports_bitrate() {
    std::cerr << "FlirOneStream::supports_bitrate()" << std::endl;
    return false;
}


void FlirOneStream::set_bitrate(int bitrate) {
    std::cerr << "FlirOneStream::set_bitrate(" << bitrate << ")" << std::endl;
}


bool FlirOneStream::supports_cbr() {
    std::cerr << "FlirOneStream::supports_cbr()" << std::endl;
    return false;
}


void FlirOneStream::set_cbr(bool enable) {
    std::cerr << "FlirOneStream::set_cbr(" << enable << ")" << std::endl;
}


std::vector<std::string> FlirOneStream::get_supported_formats() {
    std::cerr << "FlirOneStream::get_supported_formats()" << std::endl;
    return {};
}


std::string FlirOneStream::get_format() {
    std::cerr << "FlirOneStream::get_format()" << std::endl;
    return {};
}


void FlirOneStream::set_format(std::string format) {
    std::cerr << "FlirOneStream::set_format(" << format << ")" << std::endl;
}
