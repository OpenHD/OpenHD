#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <stdexcept>
#include <vector>
#include <string>

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
     * @param platform the platform we are running on
     * @param camera the camera to create the stream with
     * @param video_udp_port the udp port where rtp data is forwarded to, picked up by interface.
     */
    CameraStream(PlatformType platform, Camera &camera, uint16_t video_udp_port);

    // It is a good common programming practice to make them pure virtual
    // setup everything needed to start streaming
    virtual void setup()=0;
    // start streaming
    virtual void start()=0;
    // stop streaming
    virtual void stop()=0;
    /**
     * Create a verbose debug string about the current state of the stream
     * @return a string, can be printed to stdout or similar.
     */
    virtual std::string debug()=0;

    // expected as bits per second
    virtual bool supports_bitrate()=0;
    virtual void set_bitrate(int bitrate)=0;

    // not supported by every encoder, some USB cameras can do it but only with custom commands
    virtual bool supports_cbr()=0;
    virtual void set_cbr(bool enable)=0;

    // Change/set the video format selected by the user.
    virtual VideoFormat get_format()=0;
    virtual void set_format(VideoFormat newVideoFormat)=0;
protected:
    const PlatformType m_platform_type;
    Camera &m_camera;
    // This is the UDP port the video (for now rtp) stream is send to.
    // It then needs to be picked up, most likely by a wfb instance created by ohd-interface
    const uint16_t m_video_udp_port;
};

#endif
