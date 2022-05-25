

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include <openhd/mavlink.h>

#include "json.hpp"
#include "inja.hpp"
using namespace inja;
using json = nlohmann::json;

#include "openhd-camera.hpp"
#include "openhd-log.hpp"
#include "openhd-settings.hpp"

#include "cameramicroservice.h"
#include "camerastream.h"
#include "gstreamerstream.h"


constexpr uint8_t SERVICE_COMPID = MAV_COMP_ID_CAMERA;


CameraMicroservice::CameraMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id): Microservice(io_service, platform), m_is_air(is_air), m_unit_id(unit_id) {
    set_compid(SERVICE_COMPID);
}


void CameraMicroservice::setup() {
    std::cout << "CameraMicroservice::setup()" << std::endl;
    Microservice::setup();

    process_manifest();
    process_settings();
}


void CameraMicroservice::process_manifest() {
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

}

void CameraMicroservice::process_settings() {
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


void CameraMicroservice::configure(Camera &camera) {
    std::cerr << "Configuring camera: " << camera.type << std::endl;

    // these are all using gstreamer at the moment, but that may not be the case forever
    switch (camera.type) {
        case CameraTypeRaspberryPiCSI:
        case CameraTypeRaspberryPiVEYE:
        case CameraTypeJetsonCSI: 
        case CameraTypeIP:         
        case CameraTypeRockchipCSI:
        case CameraTypeUVC:
        case CameraTypeV4L2Loopback: {
            GStreamerStream stream(m_io_service, m_platform_type, camera, m_base_port + camera.index);
            m_camera_streams.push_back(stream);
            break;
        }
        default: {
            std::cerr << "Unknown camera type, skipping" << std::endl;
        }
    }
}


void CameraMicroservice::process_mavlink_message(mavlink_message_t msg) {
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: {
            mavlink_param_request_list_t request;
            mavlink_msg_param_request_list_decode(&msg, &request);
            break;
        }
        case MAVLINK_MSG_ID_COMMAND_LONG: {
            mavlink_command_long_t command;
            mavlink_msg_command_long_decode(&msg, &command);

            // only process commands sent to this system or broadcast to all systems
            if ((command.target_system != this->m_sysid && command.target_system != 0)) {
                return;
            }

            // only process commands sent to this component or boadcast to all components on this system
            // add compid 200 for betaflight same fix aß here https://github.com/OpenHD/Open.HD/commit/d7e9c9956601235d105196616c20d68443a73be1
            if ((command.target_component != this->m_compid && command.target_component != MAV_COMP_ID_ALL && command.target_component != 200)) {
                return;
            }

            switch (command.command) {
                case OPENHD_CMD_GET_CAMERA_SETTINGS: {
                    uint8_t brightness = 128; //(uint8_t)settings.value("brightness", 128).toUInt();
                    uint8_t contrast   = 128; //(uint8_t)settings.value("contrast", 128).toUInt();
                    uint8_t saturation = 128; //(uint8_t)settings.value("saturation", 128).toUInt();


                    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                    int len = 0;

                    // acknowledge the command first...
                    mavlink_message_t ack;
                    mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
                                                 this->m_compid, // and from this component
                                                 &ack,
                                                 OPENHD_CMD_GET_CAMERA_SETTINGS, // the command we're ack'ing
                                                 MAV_CMD_ACK_OK,
                                                 0,
                                                 0,
                                                 msg.sysid,   // address the ack to the senders system ID...
                                                 msg.compid); // ... and the senders component ID
                    len = mavlink_msg_to_send_buffer(raw, &ack);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));

                    // ... then reply
                    mavlink_message_t outgoing_msg;
                    mavlink_msg_openhd_camera_settings_pack(this->m_sysid, // mark the message as being from the local system ID
                                                            this->m_compid,  // and from this component
                                                            &outgoing_msg,
                                                            msg.sysid,
                                                            msg.compid,
                                                            brightness,
                                                            contrast,
                                                            saturation);
                    len = mavlink_msg_to_send_buffer(raw, &outgoing_msg);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));

                    break;
                }
                case OPENHD_CMD_SET_CAMERA_SETTINGS: {
                    /* this is how you would store a setting for the component when
                     * replying to a settings request
                     */
                    //QSettings settings;
                    //settings.beginGroup("OpenHDCamera");

                    //settings.setValue("brightness", (uint8_t)command.param1);
                    //settings.setValue("contrast", (uint8_t)command.param2);
                    //settings.setValue("saturation", (uint8_t)command.param3);

                    uint8_t raw[MAVLINK_MAX_PACKET_LEN];
                    int len = 0;

                    // acknowledge the command, no reply
                    mavlink_message_t ack;
                    mavlink_msg_command_ack_pack(this->m_sysid, // mark the message as being from the local system ID
                                                 this->m_compid,  // and from this component
                                                 &ack,
                                                 OPENHD_CMD_GET_CAMERA_SETTINGS, // the command we're ack'ing
                                                 MAV_CMD_ACK_OK,
                                                 0,
                                                 0,
                                                 msg.sysid, // send ack to the senders system ID...
                                                 msg.compid); // ... and the senders component ID
                    len = mavlink_msg_to_send_buffer(raw, &ack);

                    this->m_socket->async_send(boost::asio::buffer(raw, len),
                                               boost::bind(&Microservice::handle_write,
                                                           this,
                                                           boost::asio::placeholders::error));

                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void CameraMicroservice::debug_camerastream(){
    std::cerr << "debug_camerastream" << std::endl;
    for (auto & stream : m_camera_streams) {
        if(m_camera_streams.empty()) break;
        std::cerr << "size " << m_camera_streams.size() << std::endl;
        //std::cerr << +stream.index << "." +stream.stream_type << "." << +stream.data_type << std::endl;

        //GStreamerStream::CameraStream gstr;
        //stream= &gstr;
        //stream.GStreamerStream::CameraStream->debug();
    }
    std::cerr << "end of debug";
}

void CameraMicroservice::save_settings(std::vector<Camera> cameras, std::string settings_file) {
    Environment env;

    // load the camera template, we format it once for each camera and write that to the file
    std::ifstream template_file("/usr/local/share/openhd/camera.template");
    std::string template_s((std::istreambuf_iterator<char>(template_file)),
                          std::istreambuf_iterator<char>());


    std::ofstream out(settings_file);

    // now fill in the template params
    for (auto & camera : cameras) {
        json data;

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

        Template temp = env.parse(template_s.c_str());
        std::string rendered = env.render(temp, data);

        // and write this camera to the settings file
        out << rendered;
        out << "\n\n";
    }
    
    out.close();
}

