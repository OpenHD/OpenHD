#ifndef SEEKSTREAM_H
#define SEEKSTREAM_H

#include <array>
#include <stdexcept>
#include <vector>

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"

#include "camerastream.h"

#include <gst/gst.h>

// Implementation of OHD CameraStream for Seek ?! . Untested.

class SeekStream: public CameraStream {
public:
    SeekStream(PlatformType platform, Camera camera, uint16_t port);

    void setup() override;

    void start() override;
    void stop() override;

    bool supports_bitrate() override;
    void set_bitrate(int bitrate) override;

    bool supports_cbr() override;
    void set_cbr(bool enable) override;

    std::vector<std::string> get_supported_formats() override;
    std::string get_format() override;
    void set_format(std::string format) override;
};

#endif
