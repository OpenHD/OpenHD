//
// Created by consti10 on 03.05.22.
//

#include "inja.hpp"
#include "json.hpp"
#include "gstreamerstream.h"
#include "libcamerastream.h"
#include "openhd-settings.hpp"

#include "OHDVideo.h"

#include <utility>


OHDVideo::OHDVideo(bool is_air, std::string unit_id,PlatformType platform_type):m_is_air(is_air),m_unit_id(std::move(unit_id)),m_platform_type(platform_type) {
    assert(("This module must only run on the air pi !", m_is_air==true));
    try {
        setup();
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }
}

void OHDVideo::debug() const{
    // TODO make it much more verbose
    std::cerr << "OHDVideo::debug\n";
    std::cerr<<"N camera streams:"<<m_camera_streams.size()<<"\n";
    for(int i=0;i<m_camera_streams.size();i++){
        const auto& stream=m_camera_streams.at(i);
        std::cout<<"Camera stream:"<<i<<"\n";
    }
    std::cerr << "end of debug";
}

void OHDVideo::setup() {
    std::cout << "OHDVideo::setup()" << std::endl;
    process_manifest();
    process_settings();
}

void OHDVideo::process_manifest() {
    try {
        std::ifstream f("/tmp/camera_manifest");
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

            m_cameras.push_back(camera);
        }
    } catch (std::exception &ex) {
        // don't do anything, but send an error message to the user through the status service
        std::cerr << "Camera error: " << ex.what() << std::endl;
    }
    if(m_cameras.size()==0){
        std::cerr<<"Started as air but no camera(s) have been found from json\n";
        // TODO: just start a gstreamer stream that creates a white noise or something similar.
    }
}

void OHDVideo::process_settings() {
    /*
  * Then get the local settings, if there are any
  */
    std::vector<std::map<std::string, std::string> > settings;

    try {
        std::string settings_path = find_settings_path(m_is_air, m_unit_id);
        std::cerr << "settings_path: " << settings_path << std::endl;
        std::string settings_file = settings_path + "/camera.conf";
        std::cerr << "settings_file: " << settings_file << std::endl;
        settings = read_config(settings_file);
    } catch (std::exception &ex) {
        std::cerr << "Camera settings load error: " << ex.what() << std::endl;
    }


    /*
     * Now use the settings to override the detected hardware configuration in each Camera in m_cameras
     *
     */
    std::vector<Camera> save_cameras;

    for (auto camera : m_cameras) {
        std::map<std::string, std::string> setting_map;

        for (auto & settings_for_camera : settings) {
            if (settings_for_camera.count("bus") == 1 && settings_for_camera["bus"] == camera.bus) {
                setting_map = settings_for_camera;
                break;
            }
        }

        if (setting_map.count("format")) camera.format = setting_map["format"];
        if (setting_map.count("bitrate")) camera.bitrate = setting_map["bitrate"];
        if (setting_map.count("rotate")) camera.rotate = setting_map["rotate"];
        if (setting_map.count("brightness")) camera.brightness = setting_map["brightness"];
        if (setting_map.count("contrast")) camera.contrast = setting_map["contrast"];
        if (setting_map.count("sharpness")) camera.sharpness = setting_map["sharpness"];
        if (setting_map.count("wdr")) camera.wdr = setting_map["wdr"];
        if (setting_map.count("denoise")) camera.denoise = setting_map["denoise"];
        if (setting_map.count("thermal_palette")) camera.thermal_palette = setting_map["thermal_palette"];
        if (setting_map.count("thermal_span")) camera.thermal_span = setting_map["thermal_span"];
        if (setting_map.count("rc_channel_record")) camera.rc_channel_record = setting_map["rc_channel_record"];
        if (setting_map.count("url")) camera.url = setting_map["url"];
        if (setting_map.count("manual_pipeline")) camera.manual_pipeline = setting_map["manual_pipeline"];
        if (setting_map.count("codec")) camera.codec = string_to_video_codec(setting_map["codec"]);

        configure(camera);
        save_cameras.push_back(camera);
    }

    /*
     * And now save the complete set of cameras back to the settings file, ensuring that all hardware
     * ends up in the file automatically but users can change it as needed
     */
    try {
        std::string settings_path = find_settings_path(m_is_air, m_unit_id);
        std::string settings_file = settings_path + "/camera.conf";
        save_settings(save_cameras, settings_file);
    } catch (std::exception &ex) {
        ohd_log(STATUS_LEVEL_EMERGENCY, "Camera settings save failed");
    }
}

void OHDVideo::configure(Camera &camera) {
    std::cerr << "Configuring camera: " << camera_type_to_string(camera.type) << std::endl;
    // these are all using gstreamer at the moment, but that may not be the case forever
    switch (camera.type) {
        case CameraTypeRaspberryPiCSI:
        case CameraTypeRaspberryPiVEYE:
        case CameraTypeJetsonCSI:
        case CameraTypeIP:
        case CameraTypeRockchipCSI:
        case CameraTypeUVC:
        case CameraTypeV4L2Loopback: {
            std::cout<<"Camera index:"<<camera.index<<"\n";
            const auto udp_port = camera.index == 0 ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
            auto stream=std::make_unique<GStreamerStream>(m_platform_type, camera, udp_port);
            stream->setup();
            stream->start();
            m_camera_streams.push_back(std::move(stream));
            break;
        }
        default: {
            std::cerr << "Unknown camera type, skipping" << std::endl;
        }
    }
}

void OHDVideo::save_settings(std::vector<Camera> cameras, std::string settings_file) {
    inja::Environment env;

    // load the camera template, we format it once for each camera and write that to the file
    std::ifstream template_file("/usr/local/share/openhd/camera.template");
    std::string template_s((std::istreambuf_iterator<char>(template_file)),
                           std::istreambuf_iterator<char>());


    std::ofstream out(settings_file);

    // now fill in the template params
    for (auto & camera : cameras) {
        nlohmann::json data;

        data["type"] = camera_type_to_string(camera.type);
        data["bus"] = camera.bus;
        data["name"] = camera.name;
        data["vendor"] = camera.vendor;

        data["format"] = camera.format;
        data["bitrate"] = camera.bitrate;
        data["rotate"] = camera.rotate;
        data["brightness"] = camera.brightness;
        data["contrast"] = camera.contrast;
        data["sharpness"] = camera.sharpness;
        data["wdr"] = camera.wdr;
        data["denoise"] = camera.denoise;
        data["thermal_palette"] = camera.thermal_palette;
        data["thermal_span"] = camera.thermal_span;
        data["rc_channel_record"] = camera.rc_channel_record;
        data["url"] = camera.url;
        data["manual_pipeline"] = camera.manual_pipeline;
        data["codec"] = video_codec_to_string(camera.codec);

        inja::Template temp = env.parse(template_s.c_str());
        std::string rendered = env.render(temp, data);

        // and write this camera to the settings file
        out << rendered;
        out << "\n\n";
    }

    out.close();
}

