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

#include <libusb.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"

#include "DCameras.h"


 #define FLIR_ONE_VENDOR_ID 0x09cb
 #define FLIR_ONE_PRODUCT_ID 0x1996

 #define SEEK_COMPACT_VENDOR_ID 0x289d
 #define SEEK_COMPACT_PRODUCT_ID 0x0010

 #define SEEK_COMPACT_PRO_VENDOR_ID 0x289d
 #define SEEK_COMPACT_PRO_PRODUCT_ID 0x0011

DCameras::DCameras(PlatformType platform_type, BoardType board_type, CarrierType carrier_type) :
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type) {}



void DCameras::discover() {
    std::cout << "Cameras::discover()" << std::endl;

    switch (m_platform_type) {
        case PlatformTypeRaspberryPi: {
            detect_raspberrypi_csi();
            detect_raspberrypi_veye();
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

    detect_flir();
    detect_seek();

    detect_v4l2();
    detect_ip();
}



/*
 * This is used when the gpu firmware is in charge of the camera, we have to ask it. This should 
 * be the only place in the entire system that uses vcgencmd for this purpose, anything else that 
 * needs to know should read from the openhd system manifest instead.
 *
 */
void DCameras::detect_raspberrypi_csi() {
    std::cout<< "Cameras::detect_raspberrypi_csi()" << std::endl;

    std::array<char, 512> buffer{};
    std::string raw_value;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("vcgencmd get_camera", "r"), pclose);
    if (!pipe) {
        std::cout << "Cameras::detect_raspberrypi_csi() no pipe from vcgencmd" << std::endl;
        return;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        raw_value += buffer.data();
    }

    boost::smatch result;

    // example "supported=2 detected=2"
    boost::regex r{ "supported=([\\d]+)\\s+detected=([\\d]+)"};
    
    if (!boost::regex_search(raw_value, result, r)) {
        std::cout<< "Cameras::detect_raspberrypi_csi() no regex match" << std::endl;
        return;
    }

    if (result.size() != 3) {
        std::cout << "Cameras::detect_raspberrypi_csi() regex unexpected result" << std::endl;
        return;
    }

    std::string supported = result[1];
    std::string detected = result[2];

    std::cout << "Cameras::detect_raspberrypi_csi() supported="+supported+" detected="+detected << std::endl;

    auto camera_count = atoi(detected.c_str());

    if (camera_count >= 1) {
        Camera camera;
        camera.name = "Pi CSI 0";
        camera.vendor = "Raspberry Pi";
        camera.type = CameraTypeRaspberryPiCSI;
        camera.bus = "0";
        camera.index = m_discover_index;
        m_discover_index++;
        CameraEndpoint endpoint;
        endpoint.bus = camera.bus;
        endpoint.support_h264 = true;
        endpoint.support_mjpeg = false;

        // these are temporary, there isn't a way to ask the old broadcom camera drivers about the supported
        // resolutions, but we know which ones people actually use so we can simply mark them for now
        endpoint.formats.emplace_back("H.264|640x480@30");
        endpoint.formats.emplace_back("H.264|640x480@48");
        endpoint.formats.emplace_back("H.264|640x480@60");
        endpoint.formats.emplace_back("H.264|800x480@30");
        endpoint.formats.emplace_back("H.264|1280x720@30");
        endpoint.formats.emplace_back("H.264|1280x720@48");
        endpoint.formats.emplace_back("H.264|1280x720@59.9");
        endpoint.formats.emplace_back("H.264|1012x760@90");
        endpoint.formats.emplace_back("H.264|1012x760@120");
        endpoint.formats.emplace_back("H.264|1920x1080@30");
        endpoint.formats.emplace_back("H.264|1920x1080@59.9");

        m_camera_endpoints.push_back(endpoint);
        m_cameras.push_back(camera);
    }
    if (camera_count >= 2) {
        Camera camera;
        camera.name = "Pi CSI 1";
        camera.vendor = "Raspberry Pi";
        camera.type = CameraTypeRaspberryPiCSI;
        camera.bus = "1";
        camera.index = m_discover_index;
        m_discover_index++;
        CameraEndpoint endpoint;
        endpoint.bus = camera.bus;
        endpoint.support_h264 = true;
        endpoint.support_mjpeg = false;

        endpoint.formats.emplace_back("H.264|640x480@30");
        endpoint.formats.emplace_back("H.264|640x480@48");
        endpoint.formats.emplace_back("H.264|640x480@60");
        endpoint.formats.emplace_back("H.264|800x480@30");
        endpoint.formats.emplace_back("H.264|1280x720@30");
        endpoint.formats.emplace_back("H.264|1280x720@48");
        endpoint.formats.emplace_back("H.264|1280x720@59.9");
        endpoint.formats.emplace_back("H.264|1012x760@90");
        //endpoint.formats.emplace_back("H.264|1012x760@120");
        endpoint.formats.emplace_back("H.264|1920x1080@30");
        //endpoint.formats.emplace_back("H.264|1920x1080@59.9");

        m_camera_endpoints.push_back(endpoint);
        m_cameras.push_back(camera);
    }

}

void DCameras::detect_raspberrypi_veye() {
    std::cout<< "Cameras::detect_raspberrypi_veye()" << std::endl;
    
    bool success = false;

    std::vector<std::string> args { 
        "/usr/local/share/veye-raspberrypi/camera_i2c_config"
    };

    success = run_command("/bin/bash", args);

    if (!success) {
        ohd_log(STATUS_LEVEL_WARNING, "Failed to enable veye camera config");
        return;
    }

    std::array<char, 512> buffer{};
    std::string veye_detect;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("i2cdetect -y 0 0x3b 0x3b | grep  '3b'", "r"), pclose);
    if (!pipe) {
        std::cout<< "Cameras::detect_raspberrypi_veye() no pipe from i2cdetect" << std::endl;
        return;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        veye_detect += buffer.data();
    }

    std::cout<< "i2cdetect result: "+veye_detect << std::endl;

    boost::smatch result;

    boost::regex r{ "30:                                  3b            "};
    
    if (!boost::regex_search(veye_detect, result, r)) {
        std::cout<< "Cameras::detect_raspberrypi_veye() no regex match" << std::endl;
        return;
    }

    Camera camera;
    camera.name = "Pi VEYE 0";
    camera.vendor = "VEYE";
    camera.type = CameraTypeRaspberryPiVEYE;
    camera.bus = "0";
    camera.index = m_discover_index;
    m_discover_index++;
    CameraEndpoint endpoint;
    endpoint.bus = camera.bus;
    endpoint.support_h264 = true;
    endpoint.support_mjpeg = false;

    endpoint.formats.emplace_back("H.264|1920x1080@25");
    endpoint.formats.emplace_back("H.264|1920x1080@30");

    m_camera_endpoints.push_back(endpoint);
    m_cameras.push_back(camera);
    std::cout<< "Cameras::detect_raspberrypi_veye() end\n";
}



void DCameras::detect_jetson_csi() {
    std::cout<< "Cameras::detect_jetson_csi()" << std::endl;
}



void DCameras::detect_rockchip_csi() {
    std::cout<< "Cameras::detect_rockchip_csi()" << std::endl;
}



void DCameras::detect_v4l2() {
    std::cout<< "Cameras::detect_v4l2()" << std::endl;

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



void DCameras::probe_v4l2_device(const std::string& device) {
    std::cout<< "Cameras::probe_v4l2_device()" << std::endl;

    std::stringstream command;
    command << "udevadm info ";
    command << device.c_str();

    std::array<char, 512> buffer{};
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
        camera.index = m_discover_index;
        m_discover_index++;
        m_cameras.push_back(camera);
    }

    m_camera_endpoints.push_back(endpoint);
}



bool DCameras::process_video_node(Camera& camera, CameraEndpoint& endpoint, const std::string& node) {
    std::cout<< "Cameras::process_video_node(" << node << ")" << std::endl;

    int fd;
    if ((fd = v4l2_open(node.c_str(), O_RDWR)) == -1) {
        std::cout<< "Can't open: " << node << std::endl;
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
        std::cout << "Found UVC camera" << std::endl;
    } else if (driver == "tegra-video") {
        camera.type = CameraTypeJetsonCSI;
        std::cout << "Found Jetson CSI camera" << std::endl;
    } else if (driver == "v4l2 loopback") {
        // this is temporary, we are not going to use v4l2loopback for thermal cameras they'll be directly
        // handled by the camera service instead
        camera.type = CameraTypeV4L2Loopback;
        std::cout << "Found v4l2 loopback camera (likely a thermal camera)" << std::endl;
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

                        std::cout << "Found format: " << new_format.str() << std::endl;
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



void DCameras::detect_ip() {

}


void DCameras::detect_flir() {
    /*
     * What this is:
     * 
     * We're detecting whether the flir one USB thermal camera is connected. We then run the flir one driver
     * with systemd.
     *
     * What happens after:
     * 
     * The systemd service starts, finds the camera and begins running on the device node we select. Then
     * we will let it be found by the rest of this class just like any other camera, so it gets recorded 
     * in the manifest and found by the camera service.
     *
     *
     * todo: this should really be marking the camera as a thermal cam instead of starting v4l2loopback and
     *       abstracting it away like this, but the camera service doesn't yet have a thermal handling class
     */

    libusb_context *context = nullptr;
    int result = libusb_init(&context);
    if (result) {
        std::cerr << "Failed to initialize libusb" << std::endl;
        return;
    }

    libusb_device_handle *handle = libusb_open_device_with_vid_pid(nullptr, FLIR_ONE_VENDOR_ID, FLIR_ONE_PRODUCT_ID);

    if (handle) {
        std::vector<std::string> ar { 
            "start", "flirone"
        };

        boost::process::child p(boost::process::search_path("systemctl"), ar);
        p.wait();
    }
}


void DCameras::detect_seek() {
    /*
     * What this is:
     * 
     * We're detecting whether the 2 known Seek thermal USB cameras are connected, then constructing
     * arguments for the seekthermal driver depending on which model it is. We then run the seek driver
     * with systemd using the arguments file we provided to it in seekthermal.service in the libseek-thermal 
     * package.
     *
     * What happens after:
     * 
     * The systemd service starts, finds the camera and begins running on the device node we select. Then
     * we will let it be found by the rest of this class just like any other camera, so it gets recorded 
     * in the manifest and found by the camera service.
     *
     *
     * todo: this should pull the camera settings from the settings file if available
     */
    
    libusb_context *context = nullptr;
    int result = libusb_init(&context);
    if (result) {
        std::cerr << "Failed to initialize libusb" << std::endl;
        return;
    }

    libusb_device_handle *handle_compact = libusb_open_device_with_vid_pid(nullptr, SEEK_COMPACT_VENDOR_ID, SEEK_COMPACT_PRODUCT_ID);
    libusb_device_handle *handle_compact_pro = libusb_open_device_with_vid_pid(nullptr, SEEK_COMPACT_PRO_VENDOR_ID, SEEK_COMPACT_PRO_PRODUCT_ID);
    
    // todo: this will need to be pulled from the config, we may end up running these from the camera service so that
    //       it can see the camera settings, which are not visible to openhd-system early at boot
    std::string model;
    std::string fps;

    if (handle_compact) {
        std::cout << "Found seek" << std::endl;
        model = "seek";
        fps = "7";
    }

    if (handle_compact_pro) {
        std::cout<< "Found seekpro" << std::endl;
        model = "seekpro";
        // todo: this is not necessarily accurate, not all compact pro models are 15hz
        fps = "15";
    }

    if (handle_compact_pro || handle_compact) {
        std::cout<< "Found seek thermal camera" << std::endl;

        std::ofstream _u("/etc/openhd/seekthermal.conf", std::ios::binary | std::ios::out);
        // todo: this should be more dynamic and allow for multiple cameras
        _u << "DeviceNode=/dev/video4";
        _u << std::endl;
        _u << "SeekModel=";
        _u << model;
        _u << std::endl;
        _u << "FPS=";
        _u << fps;
        _u << std::endl;
        _u << "SeekColormap=11";
        _u << std::endl;
        _u << "SeekRotate=11";
        _u << std::endl;
        _u.close();

        std::vector<std::string> ar { 
            "start", "seekthermal"
        };

        boost::process::child p(boost::process::search_path("systemctl"), ar);
        p.wait();
    }
}


void DCameras::write_manifest() {
    write_camera_manifest(m_cameras,m_camera_endpoints);
}

