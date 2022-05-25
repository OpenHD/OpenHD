#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"

/**
 * Every camera stream should inherit from this class.
 * This hides away the underlying implementation (for example gstreamer,...) for different platform(s).
 * The paradigms developers should aim for with each camera stream are:
 * 1) Once an instance is created, it will start generating video data, already encoded if possible.
 * 2) If the camera disconnects or the underlying  process crashes (for whatever reason) the underlying implementation
 *    should re-start the camera and encoding process
 * 3) If the user changes camera parameters, it should store these changes locally (such that they are also set after the next
 * re-start) and apply the changes. It is no problem to just restart the underlying camera/encoding process with the new parameters.
 * 4) The implementation(s) should report if changing the various parameters is possible.
 *
 * TODO for performance, we probably want to get rid of the UDP port(s) here and instead go with a raw data callback
 * that can be dynamically added and does the bridge to wifibroadcast.
 */
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
