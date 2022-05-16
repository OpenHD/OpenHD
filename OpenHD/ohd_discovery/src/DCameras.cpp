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
#include <regex>

#include <libusb.h>

#include <boost/filesystem.hpp>

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "DThermalCamerasHelper.hpp"

#include "DCameras.h"


DCameras::DCameras(PlatformType platform_type, BoardType board_type, CarrierType carrier_type) :
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type) {}



void DCameras::discover() {
    std::cout << "Cameras::discover()" << std::endl;
    switch (m_platform_type) {
        case PlatformTypeRaspberryPi: {
            detect_raspberrypi_csi();
            //detect_raspberrypi_veye();
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
    // I think these need to be run before the detectv4l2 ones, since they are then picked up just like a normal v4l2 camera ??!!
    DThermalCamerasHelper::enableFlirIfFound();
    DThermalCamerasHelper::enableSeekIfFound();
    detect_v4l2();
    detect_ip();
}


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
    std::smatch result;
    // example "supported=2 detected=2"
    std::regex r{ R"(supported=([\d]+)\s+detected=([\d]+))"};
    if (!std::regex_search(raw_value, result, r)) {
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


void DCameras::detect_jetson_csi() {
    std::cout<< "Unimpl. Cameras::detect_jetson_csi()" << std::endl;
}


void DCameras::detect_rockchip_csi() {
    std::cout<< "Unimpl. Cameras::detect_rockchip_csi()" << std::endl;
}

std::vector<std::string> DCameras::findV4l2VideoDevices() {
    boost::filesystem::path dev("/dev");
    std::vector<std::string> ret;
    for (auto &entry : boost::filesystem::directory_iterator(dev)) {
        auto device_file = entry.path().string();
        std::smatch result;
        std::regex r{ "/dev/video([\\d]+)"};
        if (!std::regex_search(device_file, result, r)) {
            continue;
        }
        ret.push_back(entry.path().string());
    }
    return ret;
}


void DCameras::detect_v4l2() {
    std::cout<< "Cameras::detect_v4l2()" << std::endl;
    // Get all the devices to take into consideration.
    const auto devices=findV4l2VideoDevices();
    for(const auto& device:devices){
        probe_v4l2_device(device);
    }
}


void DCameras::probe_v4l2_device(const std::string& device){
    std::cout<< "Cameras::probe_v4l2_device()"<<device<<"\n";
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
    std::smatch model_result;
    std::regex model_regex{ "ID_MODEL=([\\w]+)"};
    if (std::regex_search(udev_info, model_result, model_regex)) {
        if (model_result.size() == 2) {
            camera.name = model_result[1];
        }
    }
    // check for device vendor
    std::smatch vendor_result;
    std::regex vendor_regex{ "ID_VENDOR=([\\w]+)"};
    if (std::regex_search(udev_info, vendor_result, vendor_regex)) {
        if (vendor_result.size() == 2) {
            camera.vendor = vendor_result[1];
        }
    }
    // check for vid
    std::smatch vid_result;
    std::regex vid_regex{ "ID_VENDOR_ID=([\\w]+)"};
    if (std::regex_search(udev_info, vid_result, vid_regex)) {
        if (vid_result.size() == 2) {
            camera.vid = vid_result[1];
        }
    }
    // check for pid
    std::smatch pid_result;
    std::regex pid_regex{ "ID_MODEL_ID=([\\w]+)"};
    if (std::regex_search(udev_info, pid_result, pid_regex)) {
        if (pid_result.size() == 2) {
            camera.pid = pid_result[1];
        }
    }
    CameraEndpoint endpoint;
    endpoint.device_node = device;
    if (!process_v4l2_node(device,camera,endpoint)) {
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


bool DCameras::process_v4l2_node(const std::string& node,Camera& camera, CameraEndpoint& endpoint) {
    std::cout<< "Cameras::process_v4l2_node(" << node << ")" << std::endl;
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
        // handled by the camera service instead work anyways
        // Consti10: Removed for this release, won't
        //camera.type = CameraTypeV4L2Loopback;
        std::cout << "Found v4l2 loopback camera (likely a thermal camera)" << std::endl;
        std::cerr<< "Loopback is unimplemented rn\n";
        return false;
    } else {
        /*
         * This is primarily going to be the bcm2835-v4l2 interface on the Pi, and non-camera interfaces.
         *
         * We don't want to use the v4l2 interface to the CSI hardware on Raspberry Pi yet, it offers no
         * advantage over the mmal interface and doesn't offer the same image controls. Once libcamera is
         * being widely used this will be the way to support those cameras, but not yet.
         */
        std::cerr<<"Found V4l2 device with unknown driver:"<<driver<<"\n";
        return false;
    }
    std::string bus((char*)caps.bus_info);
    camera.bus = bus;
    endpoint.bus = bus;
    if (!(caps.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
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
    std::cerr<<"Detect_ip unimplemented\n";
}

void DCameras::write_manifest() {
    // Fixup endpoints, would be better to seperate the discovery steps properly so that this is not needed
    for(auto & camera:m_cameras){
        std::vector<CameraEndpoint> endpointsForThisCamera;
        for(const auto& endpoint:m_camera_endpoints){
            if(camera.bus==endpoint.bus){
                // an endpoint who cannot do anything is just a waste and added complexity for later modules
                if(endpoint.formats.empty()){
                    // not really an error, since it is an implementation issue during detection that is negated here
                    std::cout<<"Discarding endpoint due to no formats\n";
                    continue;
                }
                if(!endpoint.supports_anything()){
                    // not really an error, since it is an implementation issue during detection that is negated here
                    std::cout<<"Discarding endpoint due to no capabilities\n";
                    continue;
                }
                endpointsForThisCamera.push_back(endpoint);
            }
        }
        camera.endpoints=endpointsForThisCamera;
        // also, a camera without a endpoint - what the heck should that be
        if(camera.endpoints.empty()){
            std::cerr<<"Warning Camera without endpoints\n";
        }
    }
    // make sure the camera indices are right
    int camIdx=0;
    for(auto& camera:m_cameras){
        camera.index=camIdx;
        camIdx++;
    }
    write_camera_manifest(m_cameras);
}


