#ifndef SEEKSTREAM_H
#define SEEKSTREAM_H

#include <array>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"

#include "camerastream.h"

#include <gst/gst.h>

class SeekStream: public CameraStream {
public:
    SeekStream(boost::asio::io_service &io_service, PlatformType platform, Camera camera, uint16_t port);

    void setup();

    void start();
    void stop();

    bool supports_bitrate();
    void set_bitrate(int bitrate);

    bool supports_cbr();
    void set_cbr(bool enable);

    std::vector<std::string> get_supported_formats();
    std::string get_format();
    void set_format(std::string format);
};

#endif
