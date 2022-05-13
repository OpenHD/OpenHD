

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/bind.hpp>

#include "seekstream.h"


SeekStream::SeekStream(PlatformType platform,
                       Camera camera, 
                       uint16_t port)
    : CameraStream(platform, camera, port) {
}


void SeekStream::setup() {
    std::cerr << "SeekStream::setup()" << std::endl;

    std::string device_node;

    // todo: this needs to use the correct thermal device node, not just the first raw stream
    for (auto &endpoint : m_camera.endpoints) {
        if (endpoint.support_raw) {
            device_node = endpoint.device_node;
            break;
        }
    }
}


void SeekStream::start() {
    std::cerr << "SeekStream::start()" << std::endl;
}


void SeekStream::stop() {
    std::cerr << "SeekStream::stop()" << std::endl;
}


bool SeekStream::supports_bitrate() {
    std::cerr << "SeekStream::supports_bitrate()" << std::endl;
    return false;
}


void SeekStream::set_bitrate(int bitrate) {
    std::cerr << "SeekStream::set_bitrate(" << bitrate << ")" << std::endl;
}


bool SeekStream::supports_cbr() {
    std::cerr << "SeekStream::supports_cbr()" << std::endl;
    return false;
}


void SeekStream::set_cbr(bool enable) {
    std::cerr << "SeekStream::set_cbr(" << enable << ")" << std::endl;
}


std::vector<std::string> SeekStream::get_supported_formats() {
    std::cerr << "SeekStream::get_supported_formats()" << std::endl;
    return {};
}


std::string SeekStream::get_format() {
    std::cerr << "SeekStream::get_format()" << std::endl;
    return {};
}


void SeekStream::set_format(std::string format) {
    std::cerr << "SeekStream::set_format(" << format << ")" << std::endl;
}
