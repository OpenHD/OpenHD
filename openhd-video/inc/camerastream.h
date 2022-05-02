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
    /**
     * After a camera stream is constructed, it won't start streaming until setup() and start() are called
     * @param io_service
     * @param platform
     * @param camera
     * @param port
     */
    CameraStream(boost::asio::io_service &io_service, PlatformType platform, Camera &camera, uint16_t port);

    // It is a good common programming practice to make them pure virtual
    virtual void setup()=0;

    virtual void start()=0;
    virtual void stop()=0;

    // expected as bits per second
    virtual bool supports_bitrate()=0;
    virtual void set_bitrate(int bitrate)=0;

    // not supported by every encoder, some USB cameras can do it but only with custom commands
    virtual bool supports_cbr()=0;
    virtual void set_cbr(bool enable)=0;

    // expected to be widthXheight@fps format
    virtual std::vector<std::string> get_supported_formats()=0;
    virtual std::string get_format()=0;
    virtual void set_format(std::string format)=0;

protected:
    boost::asio::io_service &m_io_service;

    const PlatformType m_platform_type;
    Camera &m_camera;

    const uint16_t m_video_port;

    bool m_enable_videotest = false;
};

#endif
