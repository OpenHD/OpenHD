#ifndef GSTREAMERSTREAM_H
#define GSTREAMERSTREAM_H

#include <array>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-camera.hpp"
#include "openhd-platform.hpp"

#include "camerastream.h"

#include <gst/gst.h>

class GStreamerStream: public CameraStream {
public:
    GStreamerStream(boost::asio::io_service &io_service, PlatformType platform, Camera &camera, uint16_t port);

    void setup();

    void setup_raspberrypi_csi();
    void setup_jetson_csi();
    void setup_rockchip_csi();

    void setup_usb_uvc();
    void setup_usb_uvch264();
    void setup_ip_camera();


    void start();
    void stop();

    bool supports_cbr();
    bool get_cbr();
    void set_cbr(bool enable);

    bool supports_bitrate();
    std::string get_bitrate();
    void set_bitrate(int bitrate);

    std::vector<std::string> get_supported_formats();
    std::string get_format();
    void set_format(std::string format);

    std::string get_brightness();
    void set_brightness(std::string);

    std::string get_contrast();
    void set_contrast(std::string);
private:
    GstElement * gst_pipeline = nullptr;

    GMainLoop *mainLoop = nullptr;

    std::string m_device_node;

    std::stringstream m_pipeline;

    bool parse_user_format(std::string format, std::string &width, std::string &height, std::string &fps);

    std::string find_v4l2_format(CameraEndpoint &endpoint, bool force_pixel_format, std::string pixel_format);

};

#endif
