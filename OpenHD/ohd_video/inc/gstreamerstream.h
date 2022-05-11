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

// Implementation of OHD CameraStream for pretty much everything, using gstreamer.

class GStreamerStream: public CameraStream {
public:
    GStreamerStream(PlatformType platform, Camera &camera, uint16_t video_udp_port);
    void setup() override;
private:
    void setup_raspberrypi_csi();
    void setup_raspberrypi_veye();
    void setup_jetson_csi();
    void setup_rockchip_csi();

    void setup_usb_uvc();
    void setup_usb_uvch264();
    void setup_ip_camera();

public:
    void start() override;
    void stop() override;
    void debug() override;
    
    bool supports_cbr() override;
    void set_cbr(bool enable) override;

    bool supports_bitrate() override;
    void set_bitrate(int bitrate) override;

    std::vector<std::string> get_supported_formats() override;
    std::string get_format() override;
    void set_format(std::string format) override;

    std::string get_brightness();
    void set_brightness(const std::string&);

    std::string get_contrast();
    void set_contrast(const std::string&);
private:
    GstElement * gst_pipeline = nullptr;

    GMainLoop *mainLoop = nullptr;
    // The pipeline that is started in the end
    std::stringstream m_pipeline;

    bool parse_user_format(const std::string& format, std::string &width, std::string &height, std::string &fps);
    std::string find_v4l2_format(CameraEndpoint &endpoint, bool force_pixel_format, std::string pixel_format);

};

#endif
