#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H


#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include "openhd-util.hpp"
#include "openhd-log.hpp"
#include "json.hpp"

typedef enum CameraType {
    CameraTypeRaspberryPiCSI,
    CameraTypeRaspberryPiVEYE,
    CameraTypeJetsonCSI,
    CameraTypeRockchipCSI,
    CameraTypeUVC,
    // this is not just a UVC camera that happens to support h264, it's the standard UVC H264 that only a few cameras
    // support, like the older models of the Logitech C920. Other UVC cameras may support h264, but they do it in a 
    // completely different way so we keep them separate
    CameraTypeUVCH264,
    CameraTypeIP,
    CameraTypeV4L2Loopback,
    CameraTypeDummy, // Dummy camera, is created fully in sw
    CameraTypeUnknown
} CameraType;


struct CameraEndpoint {
    std::string device_node;
    std::string bus;
    bool support_h264 = false;
    bool support_h265 = false;
    bool support_mjpeg = false;
    bool support_raw = false;

    std::vector<std::string> formats;
};

typedef enum VideoCodec {
    VideoCodecH264,
    VideoCodecH265,
    VideoCodecMJPEG,
    VideoCodecUnknown
} VideoCodec;

// Each camera should support at least one video format,
// But it might support a wide variety of video formats.
// For example,a camera might be able to do h264 and h265
// for multiple resolution@framerate tuples.
// Example: one of the supported formats of rpi cam is
// h264,1280x720@60
// TODO: what did stephen do there with the endpoints ?!!
struct VideoFormat{
    VideoCodec videoCodec;
    int width;
    int height;
    int framerate;
};

struct Camera {
    CameraType type=CameraTypeUnknown;
    std::string name = "unknown";
    std::string vendor = "unknown";
    std::string vid;
    std::string pid;
    // for USB this is the bus number, for CSI it's the connector number
    std::string bus;
    // Unique index of this camera, should start at 0
    int index=0;

    // these come from the settings system

    // this comes from the camera itself, includes width/height/fps, we will need to automate this in QOpenHD
    std::string format;

    std::string rc_channel_record = "0";

    std::string bitrate;

    std::string brightness;
    std::string contrast;
    std::string sharpness;
    std::string rotate;

    std::string wdr;
    std::string denoise;

    std::string thermal_palette;
    std::string thermal_span;

    std::string url;

    std::string manual_pipeline;

    VideoCodec codec = VideoCodecH264;

    std::vector<CameraEndpoint> endpoints;
};


static std::string camera_type_to_string(const CameraType& camera_type) {
    switch (camera_type) {
        case CameraTypeRaspberryPiCSI: {
            return "pi-csi";
        }
        case CameraTypeRaspberryPiVEYE: {
            return "pi-veye";
        }
        case CameraTypeJetsonCSI: {
            return "jetson-csi";
        }
        case CameraTypeRockchipCSI: {
            return "rockchip-csi";
        }
        case CameraTypeUVC: {
            return "uvc";
        }
        case CameraTypeUVCH264: {
            return "uvch264";
        }
        case CameraTypeIP: {
            return "ip";
        }
        case CameraTypeV4L2Loopback: {
            return "v4l2loopback";
        }
        default: {
            return "unknown";
        }
    }
}


static CameraType string_to_camera_type(const std::string& camera_type) {
    if (to_uppercase(camera_type).find(to_uppercase("pi-csi")) != std::string::npos) {
        return CameraTypeRaspberryPiCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("pi-veye")) != std::string::npos) {
        return CameraTypeRaspberryPiVEYE;
    } else if (to_uppercase(camera_type).find(to_uppercase("jetson-csi")) != std::string::npos) {
        return CameraTypeJetsonCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("rockchip-csi")) != std::string::npos) {
        return CameraTypeRockchipCSI;
    } else if (to_uppercase(camera_type).find(to_uppercase("uvc")) != std::string::npos) {
        return CameraTypeUVC;
    } else if (to_uppercase(camera_type).find(to_uppercase("uvch264")) != std::string::npos) {
        return CameraTypeUVCH264;
    } else if (to_uppercase(camera_type).find(to_uppercase("ip")) != std::string::npos) {
        return CameraTypeIP;
    } else if (to_uppercase(camera_type).find(to_uppercase("v4l2loopback")) != std::string::npos) {
        return CameraTypeV4L2Loopback;
    }

    return CameraTypeUnknown;
}


inline std::string video_codec_to_string(VideoCodec codec) {
    switch (codec) {
        case VideoCodecH264: {
            return "h264";
        }
        case VideoCodecH265: {
            return "h265";
        }
        case VideoCodecMJPEG: {
            return "mjpeg";
        }
        default: {
            return "unknown";
        }
    }
}


inline VideoCodec string_to_video_codec(const std::string& codec) {
    if (to_uppercase(codec).find(to_uppercase("h264")) != std::string::npos) {
        return VideoCodecH264;
    } else if (to_uppercase(codec).find(to_uppercase("h265")) != std::string::npos) {
        return VideoCodecH265;
    } else if (to_uppercase(codec).find(to_uppercase("mjpeg")) != std::string::npos) {
        return VideoCodecMJPEG;
    }

    return VideoCodecUnknown;
}

// TODO: Why the heck did stephen not use the endpoints member variable here ?
static nlohmann::json cameras_to_json(const std::vector<Camera>& cameras,const std::vector<CameraEndpoint>& camera_endpoints){
    nlohmann::json j;
    for (const auto &camera : cameras) {
        try {
            nlohmann::json endpoints = nlohmann::json::array();
            int endpoint_index = 0;
            for (auto &_endpoint : camera_endpoints) {
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
                    {"index",         camera.index },
                    {"endpoints",     endpoints }
            };
            std::stringstream message;
            message << "Detected camera: " << camera.name << std::endl;
            ohd_log(STATUS_LEVEL_INFO, message.str());
            j.push_back(_camera);
        } catch (std::exception &ex) {
            std::cerr << "exception: " << ex.what() << std::endl;
        }
    }
    return j;
}

static constexpr auto CAMERA_MANIFEST_FILENAME="/tmp/camera_manifest";

static void write_camera_manifest(const std::vector<Camera>& cameras,const std::vector<CameraEndpoint>& camera_endpoints){
    auto manifest=cameras_to_json(cameras,camera_endpoints);
    std::ofstream _t(CAMERA_MANIFEST_FILENAME);
    _t << manifest.dump(4);
    _t.close();
}

static std::vector<Camera> cameras_from_manifest(){
    std::vector<Camera> ret;
    try {
        std::ifstream f(CAMERA_MANIFEST_FILENAME);
        nlohmann::json j;
        f >> j;

        for (auto _camera : j) {
            std::cerr << "Processing camera_manifest" << std::endl;
            Camera camera;
            std::string camera_type = _camera["type"];
            camera.type = string_to_camera_type(camera_type);
            camera.name = _camera["name"];
            std::cerr << camera.name << std::endl;
            camera.vendor = _camera["vendor"];
            camera.vid = _camera["vid"];
            camera.pid = _camera["pid"];
            camera.bus = _camera["bus"];
            camera.index = _camera["index"];

            auto _endpoints = _camera["endpoints"];
            for (auto _endpoint : _endpoints) {
                CameraEndpoint endpoint;
                endpoint.device_node   = _endpoint["device_node"];
                endpoint.support_h264  = _endpoint["support_h264"];
                endpoint.support_h265  = _endpoint["support_h265"];
                endpoint.support_mjpeg = _endpoint["support_mjpeg"];
                endpoint.support_raw   = _endpoint["support_raw"];
                for (auto& format : _endpoint["formats"]) {
                    endpoint.formats.push_back(format);
                    std::cerr << format << std::endl;
                }
                camera.endpoints.push_back(endpoint);
            }
            ret.push_back(camera);
        }
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        std::cerr << "Camera error: " << ex.what() << std::endl;
    }
    return ret;
}
#endif
