#include <cstdio>

#include <linux/videodev2.h>
#include <libv4l2.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>


#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-status.hpp"
#include "openhd-util.hpp"

#include "cameras.h"


 #define FLIR_ONE_VENDOR_ID 0x09cb
 #define FLIR_ONE_PRODUCT_ID 0x1996

 #define SEEK_COMPACT_VENDOR_ID 0x289d
 #define SEEK_COMPACT_PRODUCT_ID 0x0010

 #define SEEK_COMPACT_PRO_VENDOR_ID 0x289d
 #define SEEK_COMPACT_PRO_PRODUCT_ID 0x0011

Cameras::Cameras(PlatformType platform_type, BoardType board_type, CarrierType carrier_type) : 
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type) {}



void Cameras::discover() {
    std::cout << "Cameras::discover()" << std::endl;

    switch (m_platform_type) {
        case PlatformTypeRaspberryPi: {
            detect_raspberrypi_csi();
            break;
        }
        case PlatformTypeJetson: {
            detect_jetson_csi();
            break;
        }
        case PlatformTypeRockchip: {
            detect_rockchip_csi();
            break;
        }
        default: {
            break;
        }
    }

    detect_v4l2();
    detect_ip();
}



/*
 * This is used when the gpu firmware is in charge of the camera, we have to ask it. This should 
 * be the only place in the entire system that uses vcgencmd for this purpose, anything else that 
 * needs to know should read from the openhd system manifest instead.
 *
 */
void Cameras::detect_raspberrypi_csi() {
    std::cerr << "Cameras::detect_raspberrypi_csi()" << std::endl;

    std::array<char, 512> buffer;
    std::string raw_value;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("vcgencmd get_camera", "r"), pclose);
    if (!pipe) {
        return;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        raw_value += buffer.data();
    }

    boost::smatch result;

    // example "supported=2 detected=2"
    boost::regex r{ "supported=([\\d]+)\\s+detected=([\\d]+)"};
    
    if (!boost::regex_search(raw_value, result, r)) {
        return;
    }

    if (result.size() != 3) {
        return;
    }

    std::string supported = result[1];
    std::string detected = result[2];

    auto camera_count = atoi(detected.c_str());

    if (camera_count >= 1) {
        Camera camera;
        camera.name = "Pi CSI 0";
        camera.vendor = "Raspberry Pi";
        camera.type = CameraTypeRaspberryPiCSI;
        camera.bus = "0";
        CameraEndpoint endpoint;
        endpoint.bus = camera.bus;
        endpoint.support_h264 = true;
        endpoint.support_mjpeg = false;

        // these are temporary, there isn't a way to ask the old broadcom camera drivers about the supported
        // resolutions, but we know which ones people actually use so we can simply mark them for now
        endpoint.formats.push_back("H.264|640x480@30");
        endpoint.formats.push_back("H.264|640x480@48");
        endpoint.formats.push_back("H.264|640x480@60");
        endpoint.formats.push_back("H.264|800x480@30");
        endpoint.formats.push_back("H.264|1280x720@30");
        endpoint.formats.push_back("H.264|1280x720@48");
        endpoint.formats.push_back("H.264|1280x720@59.9");
        endpoint.formats.push_back("H.264|1012x760@90");
        endpoint.formats.push_back("H.264|1012x760@120");
        endpoint.formats.push_back("H.264|1920x1080@30");
        endpoint.formats.push_back("H.264|1920x1080@59.9");

        m_camera_endpoints.push_back(endpoint);
        m_cameras.push_back(camera);
    }
    if (camera_count >= 2) {
        Camera camera;
        camera.name = "Pi CSI 1";
        camera.vendor = "Raspberry Pi";
        camera.type = CameraTypeRaspberryPiCSI;
        camera.bus = "1";
        CameraEndpoint endpoint;
        endpoint.bus = camera.bus;
        endpoint.support_h264 = true;
        endpoint.support_mjpeg = false;

        endpoint.formats.push_back("H.264|640x480@30");
        endpoint.formats.push_back("H.264|640x480@48");
        endpoint.formats.push_back("H.264|640x480@60");
        endpoint.formats.push_back("H.264|800x480@30");
        endpoint.formats.push_back("H.264|1280x720@30");
        endpoint.formats.push_back("H.264|1280x720@48");
        endpoint.formats.push_back("H.264|1280x720@59.9");
        endpoint.formats.push_back("H.264|1012x760@90");
        endpoint.formats.push_back("H.264|1012x760@120");
        endpoint.formats.push_back("H.264|1920x1080@30");
        endpoint.formats.push_back("H.264|1920x1080@59.9");

        m_camera_endpoints.push_back(endpoint);
        m_cameras.push_back(camera);
    }
}



void Cameras::detect_jetson_csi() {
    std::cerr << "Cameras::detect_jetson_csi()" << std::endl;
}



void Cameras::detect_rockchip_csi() {
    std::cerr << "Cameras::detect_rockchip_csi()" << std::endl;
}



void Cameras::detect_v4l2() {
    std::cerr << "Cameras::detect_v4l2()" << std::endl;

    boost::filesystem::path dev("/dev");
    for (auto &entry : boost::filesystem::directory_iterator(dev)) { 
        auto device_file = entry.path().string();

        boost::smatch result;
        boost::regex r{ "/dev/video([\\d]+)"};
        if (!boost::regex_search(device_file, result, r)) {
            continue;
        }
        
        probe_v4l2_device(entry.path().string());
    }
}



void Cameras::probe_v4l2_device(std::string device) {
    std::cerr << "Cameras::probe_v4l2_device()" << std::endl;

    std::stringstream command;
    command << "udevadm info ";
    command << device.c_str();

    std::array<char, 512> buffer;
    std::string udev_info;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.str().c_str(), "r"), pclose);
    if (!pipe) {
        return;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        udev_info += buffer.data();
    }


    Camera camera;

    // check for device name
    boost::smatch model_result;
    boost::regex model_regex{ "ID_MODEL=([\\w]+)"};
    if (boost::regex_search(udev_info, model_result, model_regex)) {
        if (model_result.size() == 2) {
            camera.name = model_result[1];
        }
    }

    // check for device vendor
    boost::smatch vendor_result;
    boost::regex vendor_regex{ "ID_VENDOR=([\\w]+)"};
    if (boost::regex_search(udev_info, vendor_result, vendor_regex)) {
        if (vendor_result.size() == 2) {
            camera.vendor = vendor_result[1];
        }
    }

    // check for vid
    boost::smatch vid_result;
    boost::regex vid_regex{ "ID_VENDOR_ID=([\\w]+)"};
    if (boost::regex_search(udev_info, vid_result, vid_regex)) {
        if (vid_result.size() == 2) {
            camera.vid = vid_result[1];
        }
    }

    // check for pid
    boost::smatch pid_result;
    boost::regex pid_regex{ "ID_MODEL_ID=([\\w]+)"};
    if (boost::regex_search(udev_info, pid_result, pid_regex)) {
        if (pid_result.size() == 2) {
            camera.pid = pid_result[1];
        }
    }

    CameraEndpoint endpoint;
    endpoint.device_node = device;

    if (!process_video_node(camera, endpoint, device)) {
        return;
    }

    bool found = false;
    for (auto &stored_camera : m_cameras) {
        if (stored_camera.bus == camera.bus) {
            found = true;
        }
    }

    if (!found) {
        m_cameras.push_back(camera);
    }

    m_camera_endpoints.push_back(endpoint);
}



bool Cameras::process_video_node(Camera& camera, CameraEndpoint& endpoint, std::string node) {
    std::cerr << "Cameras::process_video_node(" << node << ")" << std::endl;

    int fd;
    if ((fd = v4l2_open(node.c_str(), O_RDWR)) == -1) {
        std::cerr << "Can't open: " << node << std::endl;
        return false;
    }

    struct v4l2_capability caps = {};
    if (ioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) {
        std::cerr << "Capability query failed: " << node << std::endl;
        return false;
    }

    std::string driver((char*)caps.driver);


    if (driver == "uvcvideo") {
        camera.type = CameraTypeUVC;
        std::cerr << "Found UVC camera" << std::endl;
    } else if (driver == "tegra-video") {
        camera.type = CameraTypeJetsonCSI;
        std::cerr << "Found Jetson CSI camera" << std::endl;
    } else {
        /*
         * This is primarily going to be the bcm2835-v4l2 interface on the Pi, and non-camera interfaces.
         * 
         * We don't want to use the v4l2 interface to the CSI hardware on Raspberry Pi yet, it offers no 
         * advantage over the mmal interface and doesn't offer the same image controls. Once libcamera is 
         * being widely used this will be the way to support those cameras, but not yet.
         */
        return false;
    }


    std::string bus((char*)caps.bus_info);

    camera.bus = bus;
    endpoint.bus = bus;

    if (!caps.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        std::cerr << "Not a capture device: " << node << std::endl;
        return false;
    }

    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;


    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {    
        frmsize.pixel_format = fmtdesc.pixelformat;
        frmsize.index = 0;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                frmival.index = 0;
                frmival.pixel_format = fmtdesc.pixelformat;
                frmival.width = frmsize.discrete.width;
                frmival.height = frmsize.discrete.height;

                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
                    if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {

                        std::stringstream new_format;

                        if (fmtdesc.pixelformat == V4L2_PIX_FMT_H264) {
                            endpoint.support_h264 = true;
                        }
                        #if defined V4L2_PIX_FMT_H265
                        else if (fmtdesc.pixelformat == V4L2_PIX_FMT_H265) {
                            endpoint.support_h265 = true;
                        }
                        #endif
                        else if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG) {
                            endpoint.support_mjpeg = true;
                        }
                        else {
                            // if it supports something else it's one of the raw formats, the camera service will
                            // figure out what to do with it
                            endpoint.support_raw = true;
                        }
                        
                        new_format << fmtdesc.description;
                        new_format << "|";
                        new_format << frmsize.discrete.width;
                        new_format << "x";
                        new_format << frmsize.discrete.height;
                        new_format << "@";
                        new_format << frmival.discrete.denominator;

                        endpoint.formats.push_back(new_format.str());

                        std::cerr << "Found format: " << new_format.str() << std::endl;
                    }

                    frmival.index++;
                }
            }

            frmsize.index++;
        }
        
        fmtdesc.index++;
    }

    v4l2_close(fd);

    return true;
}



void Cameras::detect_ip() {

}



nlohmann::json Cameras::generate_manifest() {
    nlohmann::json j = nlohmann::json::array();

    for (auto &camera : m_cameras) {
        try {

            nlohmann::json endpoints = nlohmann::json::array();

            int endpoint_index = 0;

            for (auto &_endpoint : m_camera_endpoints) {
                if (_endpoint.bus != camera.bus) {
                    continue;
                }

                endpoints[endpoint_index] = {
                    {"device_node",   _endpoint.device_node },
                    {"support_h264",  _endpoint.support_h264 },
                    {"support_h265",  _endpoint.support_h265 },
                    {"support_mjpeg", _endpoint.support_mjpeg },
                    {"support_raw",   _endpoint.support_raw },
                    {"formats",       _endpoint.formats }
                };

                endpoint_index++;
            }

            nlohmann::json _camera = { 
                {"type",          camera_type_to_string(camera.type) }, 
                {"name",          camera.name },
                {"vendor",        camera.vendor },
                {"vid",           camera.vid },
                {"pid",           camera.pid },
                {"bus",           camera.bus },
                {"endpoints",     endpoints }
            };

            std::ostringstream message;
            message << "Detected camera: " << camera.name << std::endl;

            status_message(STATUS_LEVEL_INFO, message.str());


            j.push_back(_camera);
        } catch (std::exception &ex) {
            std::cerr << "exception: " << ex.what() << std::endl;
        }
    }

    return j;
}

