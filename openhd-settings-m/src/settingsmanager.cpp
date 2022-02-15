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


constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_USER7;

std::string get_openhd_version() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("dpkg-query --showformat='${Version}' --show openhd", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Checking openhd version failed");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

SettingsManager::SettingsManager(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id):
		Microservice(io_service, platform), m_udp_socket(io_service), m_is_air(is_air), m_unit_id(unit_id) {
    set_compid(SERVICE_COMPID);    
}



void SettingsManager::setup() {
    std::cout << "SettingsManager::setup()" << std::endl;

    try {
        m_openhd_version = get_openhd_version();
    } catch (std::exception& ex) {
        // the exception itself doesn't matter, will just mean the m_openhd_version default
        // gets sent to QOpenHD
    }


    Microservice::setup();
    SettingsManager::get_air_settings();
    SettingsManager::send_settings();
    SettingsManager::settings_listener();
}


void SettingsManager::get_air_settings() {
    std::cout << "SettingsManager::send_settings()" << std::endl;
    //todo read the air settings that ground does not have

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

//TODO for testing as test if file is read 
std::cout << "cam bitrate=" << camera.bitrate << std::endl;
std::cout << "cam codec=" << camera.codec << std::endl;
    }
}


void SettingsManager::send_settings() {
    std::cout << "SettingsManager::send_settings()" << std::endl;
//TODO if air this will send the unique air settings on a timer
//once ground acks that it recieved the settings this will stop
}


void SettingsManager::settings_listener() {
    std::cout << "SettingsManager::settings_listener()" << std::endl;
    //TODO this watches for settings changes. If detected then 
    //triggers air settings to be sent again
}

