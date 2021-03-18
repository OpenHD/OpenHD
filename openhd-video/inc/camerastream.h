#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"

class CameraStream {
public:
    CameraStream(boost::asio::io_service &io_service, PlatformType platform, Camera &camera, uint16_t port);

    virtual void setup();

    virtual void start();
    virtual void stop();

    // expected as bits per second
    virtual bool supports_bitrate();
    virtual void set_bitrate(int bitrate);

    // not supported by every encoder, some USB cameras can do it but only with custom commands
    virtual bool supports_cbr();
    virtual void set_cbr(bool enable);

    // expected to be widthXheight@fps format
    std::vector<std::string> get_supported_formats();
    virtual std::string get_format();
    virtual void set_format(std::string format);

protected:
    boost::asio::io_service &m_io_service;

    PlatformType m_platform_type;
    Camera &m_camera;

    uint16_t m_video_port;

    bool m_enable_videotest = false;
};

#endif
