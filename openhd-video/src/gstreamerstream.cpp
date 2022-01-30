

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include <gst/gst.h>

#include <fmt/core.h>

#include "openhd-status.hpp"

#include "gstreamerstream.h"



GStreamerStream::GStreamerStream(boost::asio::io_service &io_service, 
                                 PlatformType platform,
                                 Camera &camera, 
                                 uint16_t port)
    : CameraStream(io_service, platform, camera, port) {
    std::cerr << "GStreamerStream::GStreamerStream()" << std::endl;

    setup();
    start();
}


/*
 * This constructs the camera pipeline automatically from the detected hardware capabilities. If a settings entry
 * in camera.conf is found for a detected camera (matched using the "bus" field), the settings override the defaults for
 * that kind of camera.
 *
 * IP cameras are handled specially because we have no way to guarantee they can be autodetected yet, and some may not be
 * detectable automatically at all. So for those we attempt to start a pipeline for any camera entry in the settings file
 * marked as an IP camera, using the URL provided by the user.
 *
 */
void GStreamerStream::setup() {
    std::cerr << "GStreamerStream::setup()" << std::endl;

    GError* error = nullptr;

    if (!gst_init_check(nullptr, nullptr, &error)) {
        std::cerr << "gst_init_check() failed: " << error->message << std::endl;
        g_error_free(error);

        throw std::runtime_error("GStreamer initialization failed");
    }

    std::cerr << "Creating GStreamer pipeline" << std::endl;

    // check to see if the user set a bitrate
    if (m_camera.bitrate.empty()) {
        m_camera.bitrate = "5000000";
    }

    // check to see if the user configured a specific video codec
    if (m_camera.codec == VideoCodecUnknown) {
        m_camera.codec = VideoCodecH264;
    }

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            setup_raspberrypi_csi();
            break;
        }
        case CameraTypeJetsonCSI: {
            setup_jetson_csi();
            break;
        }
        case CameraTypeRockchipCSI: {
            setup_rockchip_csi();
            break;
        }
        case CameraTypeUVC: {
            setup_usb_uvc();
            break;
        }
        case CameraTypeV4L2Loopback: {
            setup_usb_uvc();
            break;
        }
        case CameraTypeUVCH264: {
            setup_usb_uvch264();
            break;
        }
        case CameraTypeIP: {
            setup_ip_camera();
            break;
        }
        case CameraTypeUnknown: {
            std::cerr << "Unknown camera type" << std::endl;
            return;
        }
    }


    m_pipeline << "queue ! ";  


    if (m_camera.codec == VideoCodecH265) {
        m_pipeline << "h265parse config-interval=-1 ! ";
        m_pipeline << "rtph265pay mtu=1024 ! ";
    } else if (m_camera.codec == VideoCodecMJPEG) {
        m_pipeline << "jpegparse config-interval=-1 ! ";
        m_pipeline << "rtpjpegpay mtu=1024 ! ";
    } else {
        m_pipeline << "h264parse config-interval=-1 ! ";
        m_pipeline << "rtph264pay mtu=1024 ! ";
    }


    /*
     * Allow users to write the first part manually if they want to, we take care of the sink element because the port 
     * depends on which camera index this was and where the stream is going, which users won't know how to configure themselves.
     */
    if (!m_camera.manual_pipeline.empty()) {
        m_pipeline.clear();
        m_pipeline << m_camera.manual_pipeline;
    }

    
    m_pipeline << "tee name=t ! queue ! ";

    m_pipeline << fmt::format("udpsink host=127.0.0.1 port={} t. ! ", m_video_port);

    m_pipeline << "queue ! ";


    // this directs the video stream back to this system for recording in the Record class
    m_pipeline << fmt::format("udpsink host=127.0.0.1 port={}", m_video_port - 10);


    std::cerr << "Pipeline: " << m_pipeline.str() << std::endl;

    gst_pipeline = gst_parse_launch(m_pipeline.str().c_str(), &error);


    if (error) {
        std::cerr << "Failed to create pipeline: " << error->message << std::endl;
        return;
    }

    // todo: send a microservice channel message to inform the recording class, the ground station, and any connected apps
    //       about the camera codec in use

    //GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE(gst_pipeline));
    //gst_bus_add_signal_watch(bus);
    //g_signal_connect(bus, "message", (GCallback)PipelineCb, this);
}


bool GStreamerStream::parse_user_format(std::string format, std::int &width, std::int &height, std::string &fps) {
    boost::smatch result;
    boost::regex reg{ "([\\w\\s\\/\\-\\:])*\\|(\\d)*x(\\d)*\\@(\\d)*"};
    if (boost::regex_search(format, result, reg)) {
        if (result.size() == 4) {
            width = result[2];
            height = result[3];
            fps = result[4];

            return true;
        }
    }

    return false;
}


/*
 * This is used to pick a default based on the hardware format
 */
std::string GStreamerStream::find_v4l2_format(CameraEndpoint &endpoint, bool force_pixel_format, std::string pixel_format) {
    std::string width = "1280";
    std::string height = "720";
    std::string fps = "30";

    std::vector<std::string> search_order = {
        "1920x1080@60",
        "1920x1080@30",
        "1280x720@60",
        "1280x720@30",
        "800x600@30",
        "640x480@30",
        "320x240@30"
    };

    for (auto & default_format : search_order) {
        for (auto & format : endpoint.formats) {
            boost::smatch result;
            boost::regex reg{ "([\\w\\d\\s\\-\\:\\/])*\\|(\\d)*x(\\d)*\\@(\\d)*"};
            if (boost::regex_search(format, result, reg)) {
                if (result.size() == 4) {
                    auto c = fmt::format("{}x{}@{}", width, height, fps);

                    if (force_pixel_format) {
                        if (result[1] == pixel_format && c == default_format) {
                            width = result[2];
                            height = result[3];
                            fps = result[4];
                            
                            return fmt::format("{}x{}@{}", width, height, fps);
                        }
                    } else if (c == default_format) {
                        width = result[2];
                        height = result[3];
                        fps = result[4];

                        return fmt::format("{}x{}@{}", width, height, fps);
                    }
                }
            }
        }
    }

    // fallback using the default above
    return fmt::format("{}x{}@{}", width, height, fps);
}


void GStreamerStream::setup_raspberrypi_csi() {
    std::cerr << "Setting up Raspberry Pi CSI camera" << std::endl;

    if (m_camera.format.empty()) {
        m_camera.format = "1280x720@48";
    }
  
    std::string width;
    std::string height;
    std::string fps;
    parse_user_format(m_camera.format, width, height, fps);

    m_pipeline << fmt::format("rpicamsrc name=bitratectrl camera-number={} bitrate={} preview=0 ! ", m_camera.bus, m_camera.bitrate);
    m_pipeline << fmt::format("video/x-h264, profile=constrained-baseline, width={}, height={}, framerate={}/1, level=3.0 ! ", width, height, fps);
}


void GStreamerStream::setup_jetson_csi() {
    std::cerr << "Setting up Jetson CSI camera" << std::endl;

    // if there's no endpoint this isn't a runtime bug but a programming error in the system service,
    // because it never found an endpoint to use for some reason
    auto endpoint = m_camera.endpoints.front();


    int sensor_id = -1;

    boost::smatch result;
    boost::regex reg{ "/dev/video([\\d])"};
    if (boost::regex_search(endpoint.device_node, result, reg)) {
        if (result.size() == 2) {
            std::string s = result[1];
            sensor_id = std::stoi(s);
        }
    }

    if (sensor_id == -1) {
        status_message(STATUS_LEVEL_CRITICAL, "Failed to determine Jetson CSI sensor ID");
        return;
    }


    if (m_camera.format.empty()) {
        m_camera.format = "1280x720@48";
    }
  
    std::int width;
    std::int height;
    std::string fps = "48";
    parse_user_format(m_camera.format, width, height, fps);


    m_pipeline << fmt::format("nvarguscamerasrc do-timestamp=true sensor-id={} ! ", sensor_id);
    m_pipeline << fmt::format("video/x-raw(memory:NVMM), width={}, height={}, format=NV12, framerate={}/1 ! ", width, height, fps);
    m_pipeline << "queue ! ";

    if (m_camera.codec == VideoCodecH265) {
        m_pipeline << fmt::format("nvv4l2h265enc name=vnenc control-rate=1 insert-sps-pps=1 bitrate={} ! ", m_camera.bitrate);
    } else if (m_camera.codec == VideoCodecMJPEG) {
        m_pipeline << fmt::format("nvjpegenc quality=50 ! ");
    } else {
        m_pipeline << fmt::format("nvv4l2h264enc name=nvenc control-rate=1 insert-sps-pps=1 bitrate={} ! ", m_camera.bitrate);
    }
}


void GStreamerStream::setup_rockchip_csi() {
    std::cerr << "Setting up Rockchip CSI camera" << std::endl;


    if (m_camera.format.empty()) {
        m_camera.format = "1280x720@30";
    }

    std::string width = "1280";
    std::string height = "720";
    std::string fps = "48";
    parse_user_format(m_camera.format, width, height, fps);


    m_pipeline << fmt::format("rkisp num-buffers=512 device={} io-mode=1 ! ", m_camera.bus);
    m_pipeline << fmt::format("video/x-raw, format=NV12, width={}, height={}, format=NV12, framerate={}/1 ! ", width, height, fps);
    m_pipeline << "queue ! ";
    m_pipeline << fmt::format("mpph264enc name=mppenc bitrate={} ! ", m_camera.bitrate);
}


void GStreamerStream::setup_usb_uvc() {
    std::cerr << "Setting up USB camera" << std::endl;

    std::string device_node;

    for (auto &endpoint : m_camera.endpoints) {
        if (m_camera.codec == VideoCodecH264 && endpoint.support_h264) {
            device_node = endpoint.device_node;
            m_pipeline << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);


            if (m_camera.format.empty()) {
                m_camera.format = "1280x720@30";
            }

            std::string width = "1280";
            std::string height = "720";
            std::string fps = "30";
            auto have_format = parse_user_format(m_camera.format, width, height, fps);

            m_pipeline << fmt::format("video/x-h264, width={}, height={}, framerate={}/1 ! ", width, height, fps);

            break;
        }

        if (m_camera.codec == VideoCodecMJPEG && endpoint.support_mjpeg) {
            device_node = endpoint.device_node;
            m_pipeline << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);


            if (m_camera.format.empty()) {
                m_camera.format = "1280x720@30";
            }

            std::string width = "1280";
            std::string height = "720";
            std::string fps = "30";
            auto have_format = parse_user_format(m_camera.format, width, height, fps);

            m_pipeline << fmt::format("image/jpeg, width={}, height={}, framerate={}/1 ! ", width, height, fps);

            break;
        }
    }


    /*
     * No H264 or MJPEG endpoint was found/chosen, so we do YUV encoding. Most of these will be thermal cameras and 
     * won't be handled like this very long because support for them will be in a different class with thermal 
     * span and pallete support. However once in a while people do connect YUV webcams for testing purposes, and this
     * code supports those too.
     */
    if (device_node.empty()) {
        for (auto &endpoint : m_camera.endpoints) {
            if (endpoint.support_raw) {
                device_node = endpoint.device_node;

                m_pipeline << fmt::format("v4l2src name=picturectrl device={} ! ", device_node);

                /*
                 * This looks a little weird but if the user didn't supply a format we don't want to pick one, we'll get it
                 * wrong most of the time and letting gstreamer pick will generally work out
                 */
                std::string width;
                std::string height;
                std::string fps;
                auto have_format = parse_user_format(m_camera.format, width, height, fps);

                if (have_format) {
                    std::cerr << "Found format: " << m_camera.format << std::endl;
                    m_pipeline << fmt::format("video/x-raw, width={}, height={}, framerate={}/1 ! ", width, height, fps);
                } else {
                    std::cerr << "Allowing gstreamer to choose UVC format" << std::endl;
                    m_pipeline << fmt::format("video/x-raw ! ");
                }

                m_pipeline << "videoconvert ! ";

                switch (m_platform_type) {
                    case PlatformTypeRaspberryPi: {
                        // Pi has no h265 encoder yet, but this will support it when that happens
                        if (m_camera.codec == VideoCodecH265) {
                            m_pipeline << fmt::format("v4l2h265enc name=encodectrl bitrate={} ! ", m_camera.bitrate);
                        } else {
                            m_pipeline << fmt::format("v4l2h264enc name=encodectrl bitrate={} ! ", m_camera.bitrate);
                        }
                        break;
                    }
                    case PlatformTypeJetson: {
                        m_pipeline << "nvvidconv ! ";
                        
                        // todo: this requires tuning for latency and image quality
                        if (m_camera.codec == VideoCodecH265) {
                            m_pipeline << fmt::format("nvv4l2h265enc name=encodectrl control-rate=1 insert-sps-pps=1 bitrate={} ! ", m_camera.bitrate);
                        } else {
                            m_pipeline << fmt::format("nvv4l2h264enc name=encodectrl control-rate=1 insert-sps-pps=1 bitrate={} ! ", m_camera.bitrate);
                        }

                        break;
                    }
                    case PlatformTypeRockchip: {
                        m_pipeline << fmt::format("mpph264enc name=encodectrl bitrate={} ! ", m_camera.bitrate);
                        break;
                    }
                    default: {
                        if (m_camera.codec == VideoCodecH265) {
                            m_pipeline << fmt::format("x265enc name=encodectrl bitrate={} ! ", m_camera.bitrate);
                        } else {
                            m_pipeline << fmt::format("x264enc name=encodectrl bitrate={} ! ", m_camera.bitrate);
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}


void GStreamerStream::setup_usb_uvch264() {
    std::cerr << "Setting up UVC H264 camera" << std::endl;

    auto endpoint = m_camera.endpoints.front();

    if (m_camera.format.empty()) {
        m_camera.format = "1920x1080@30";
    }

    std::string width = "1920";
    std::string height = "1080";
    std::string fps = "30";
    auto have_format = parse_user_format(m_camera.format, width, height, fps);

    // uvch265 cameras don't seem to exist, codec setting is ignored
    m_pipeline << fmt::format("uvch264src device={} peak-bitrate={} initial-bitrate={} average-bitrate={} rate-control=1 iframe-period=1000 name=encodectrl auto-start=true encodectrl.vidsrc ! ", endpoint.device_node, m_camera.bitrate, m_camera.bitrate, m_camera.bitrate);
    m_pipeline << fmt::format("video/x-h264,width={}, height={}, framerate={}/1 ! ", width, height, fps);
}


void GStreamerStream::setup_ip_camera() {
    std::cerr << "Setting up IP camera" << std::endl;

    if (m_camera.url.empty()) {
        m_camera.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
    }

    // none of the other params are used at the moment, we would need to set them with ONVIF or a per-camera API of some sort,
    // however most people seem to set them in advance with the proprietary app that came with the camera anyway
    m_pipeline << fmt::format("rtspsrc location=\"{}\" latency=0 ! ", m_camera.url);
}


void GStreamerStream::start() {
    std::cerr << "GStreamerStream::start()" << std::endl;
    
    gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
}


void GStreamerStream::stop() {
    std::cerr << "GStreamerStream::stop()" << std::endl;

    gst_element_set_state(gst_pipeline, GST_STATE_PAUSED);
}


bool GStreamerStream::supports_bitrate() {
    std::cerr << "GStreamerStream::supports_bitrate()" << std::endl;

    GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");

    return (ctrl != nullptr);
}


void GStreamerStream::set_bitrate(int bitrate) {
    std::cerr << "GStreamerStream::set_bitrate(" << bitrate << ")" << std::endl;

    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring bitrate command" << std::endl;
        return;
    }

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "bitrate", bitrate, NULL);
            break;
        }
        case CameraTypeJetsonCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "bitrate", bitrate, NULL);
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "bitrate", bitrate, NULL);
            break;
        }
        case CameraTypeUVCH264: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "initial-bitrate", bitrate, NULL);
            g_object_set(ctrl, "average-bitrate", bitrate, NULL);
            break;
        }
        case CameraTypeIP: {
            // this is either going to require onvif or manually written scripting, most people are likely
            // to set it beforehand with whatever proprietary tool or web interface the camera provides
            break;
        }
        default: {
            break;
        }
    }
}


bool GStreamerStream::supports_cbr() {
    std::cerr << "GStreamerStream::supports_cbr()" << std::endl;

    // todo: this is not necessarily true, some USB and IP cameras support it but on an IP camera it may not be easy to
    //       set
    return m_camera.type == CameraTypeRaspberryPiCSI || m_camera.type == CameraTypeJetsonCSI || m_camera.type == CameraTypeUVCH264;
}


void GStreamerStream::set_cbr(bool enable) {
    std::cerr << "GStreamerStream::set_cbr(" << enable << ")" << std::endl;

    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring cbr command" << std::endl;
        return;
    }

    GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
    if (!ctrl) return;

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            break;
        }
        case CameraTypeJetsonCSI: {
            g_object_set(ctrl, "control-rate", enable ? 1 : 0, NULL);
            break;
        }
        case CameraTypeUVCH264: {
            g_object_set(ctrl, "rate-control", enable ? 1 : 0, NULL);
            break;
        }
        default: {
            break;
        }
    }
}


std::vector<std::string> GStreamerStream::get_supported_formats() {
    std::cerr << "GStreamerStream::get_supported_formats()" << std::endl;

    std::vector<std::string> formats;

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            auto endpoint = m_camera.endpoints.front();
            formats = endpoint.formats;
            break;
        }
        case CameraTypeJetsonCSI: {
            auto endpoint = m_camera.endpoints.front();
            formats = endpoint.formats;
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            for (auto &endpoint : m_camera.endpoints) {
                if (m_camera.codec == VideoCodecH265 && endpoint.support_h265) {
                    formats = endpoint.formats;
                    break;
                }
                if (m_camera.codec == VideoCodecH264 && endpoint.support_h264) {
                    formats = endpoint.formats;
                    break;
                }
                if (m_camera.codec == VideoCodecMJPEG && endpoint.support_mjpeg) {
                    formats = endpoint.formats;
                    break;
                }
                if (endpoint.support_raw) {
                    formats = endpoint.formats;
                    break;
                }
            }
            break;
        }
        case CameraTypeUVCH264: {
            auto endpoint = m_camera.endpoints.front();
            formats = endpoint.formats;
            break;
        }
        case CameraTypeIP: {
            // this is either going to require onvif or manually written scripting, most people are likely
            // to set it beforehand with whatever proprietary tool or web interface the camera provides
            break;
        }
        default: {
            break;
        }
    }

    return formats;
}


std::string GStreamerStream::get_format() {
    std::cerr << "GStreamerStream::get_format()" << std::endl;

    return m_camera.format;
}


void GStreamerStream::set_format(std::string format) {
    std::cerr << "GStreamerStream::set_format(" << format << ")" << std::endl;

    m_camera.format = format;
}


std::string GStreamerStream::get_brightness() {
    std::cerr << "GStreamerStream::get_brightness()" << std::endl;

    gint _brightness = 0;


    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring brightness request" << std::endl;
        return std::to_string(_brightness);
    }

    
    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "brightness", &_brightness, NULL);
            break;
        }
        case CameraTypeJetsonCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "brightness", &_brightness, NULL);
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "picturectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "brightness", &_brightness, NULL);
            break;
        }
        default: {
            break;
        }
    }

    return std::to_string(_brightness);
}


void GStreamerStream::set_brightness(std::string brightness) {
    std::cerr << "GStreamerStream::set_brightness(" << brightness << ")" << std::endl;

    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring brightness command" << std::endl;
        return;
    }

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_set(ctrl, "brightness", atoi(brightness.c_str()), NULL);
            break;
        }
        case CameraTypeJetsonCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_set(ctrl, "brightness", atoi(brightness.c_str()), NULL);
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "picturectrl");
            if (!ctrl) break;
            g_object_set(ctrl, "brightness", atoi(brightness.c_str()), NULL);
            break;
        }
        default: {
            break;
        }
    }
}


std::string GStreamerStream::get_contrast() {
    std::cerr << "GStreamerStream::get_contrast()" << std::endl;

    gint _contrast = 0;

    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring contrast request" << std::endl;
        return std::to_string(_contrast);
    }

    
    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "contrast", &_contrast, NULL);
            break;
        }
        case CameraTypeJetsonCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "contrast", &_contrast, NULL);
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "picturectrl");
            if (!ctrl) break;
            g_object_get(ctrl, "contrast", &_contrast, NULL);
            break;
        }
        default: {
            break;
        }
    }

    return std::to_string(_contrast);
}


void GStreamerStream::set_contrast(std::string contrast) {
    std::cerr << "GStreamerStream::set_contrast(" << contrast << ")" << std::endl;

    if (!gst_pipeline) {
        std::cerr << "No pipeline, ignoring brightness command" << std::endl;
        return;
    }

    switch (m_camera.type) {
        case CameraTypeRaspberryPiCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "contrast", atoi(contrast.c_str()), NULL);
            break;
        }
        case CameraTypeJetsonCSI: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "encodectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "contrast", atoi(contrast.c_str()), NULL);
            break;
        }
        case CameraTypeV4L2Loopback:
        case CameraTypeUVC: {
            GstElement *ctrl = gst_bin_get_by_name(GST_BIN(gst_pipeline), "picturectrl");
            if (!ctrl) return;
            g_object_set(ctrl, "contrast", atoi(contrast.c_str()), NULL);
            break;
        }
        default: {
            break;
        }
    }
}
