#include "../inc/settingsmanager.h"

#include <array>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/reboot.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <boost/regex.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <openhd/mavlink.h>

#include "openhd-platform.hpp"

#include "openhd-settings.hpp"
#include "openhd-status.hpp"

#include "../../openhd-common/openhd-status.hpp"

#include "openhd-camera.hpp"

#include "json.hpp"
#include "inja.hpp"
using namespace inja;
using json = nlohmann::json;


constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER7;


SettingsManager::SettingsManager(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id):
		Microservice(io_service, platform), m_udp_socket(io_service), m_is_air(is_air), m_unit_id(unit_id) {
    set_compid(SERVICE_COMPID);    
}



void SettingsManager::setup() {
    std::cout << "SettingsManager::setup()" << std::endl;

    Microservice::setup();

    if (m_is_air) {
        std::cout << "SettingsManager::we are Air!" << std::endl;
        SettingsManager::read_air_settings();
       // SettingsManager::send_air_settings();
    }
    else {
        std::cout << "SettingsManager::we are Ground!" << std::endl;
        //listen for air settings
    }

    SettingsManager::settings_listener();
}


void SettingsManager::read_air_settings() {
    std::cout << "SettingsManager::read_air_settings()" << std::endl;
    //todo read the air settings that ground does not have




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






    std::vector<std::map<std::string, std::string> > settings;

    try {
        std::string settings_path = find_settings_path(m_is_air, m_unit_id);
        std::cerr << "settings_path: " << settings_path << std::endl;
//TOunDO for testing the path is changed.. FIX LATER = settings_path + "/camera.conf"
        std::string settings_file =  "/home/pilotnbr1/Downloads/camera.conf";
//TOunDO        std::cerr << "settings_file: " << settings_file << std::endl;
        settings = read_config("/home/pilotnbr1/Downloads/camera.conf");
    } catch (std::exception &ex) {
        std::cerr << "Camera settings load error: " << ex.what() << std::endl;
    }


    for (auto camera : m_cameras) {
        std::map<std::string, std::string> setting_map;

        std::cout << "Found Camera in Conf file" << std::endl;

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

//TODO for testing as test if file is read 
std::cout << "cam bitrate=" << camera.bitrate << std::endl;
std::cout << "cam codec=" << camera.codec << std::endl;

send_air_settings(camera);
    }
}


void SettingsManager::send_air_settings(Camera &camera) {
    std::cout << "SettingsManager::send_air_settings()" << std::endl;
//TODO if air this will send the unique air settings on a timer
//once ground acks that it recieved the settings this will stop

std::cout << "Camera vendor=" << camera.vendor <<std::endl;
std::cout << "Camera name=" << camera.name <<std::endl;
std::cout << "Camera type=" << camera.type <<std::endl;
std::cout << "Camera vid=" << camera.vid <<std::endl;



}


void SettingsManager::settings_listener() {
    std::cout << "SettingsManager::settings_listener()" << std::endl;
    //TODO this watches for settings changes. If detected then 
    //triggers air settings to be sent again
}

