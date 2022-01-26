#include <algorithm>
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <openhd/mavlink.h>
#include <mavlink_types.h>

#include "json.hpp"
#include "inja.hpp"
using namespace inja;
using json = nlohmann::json;

#include <fmt/core.h>

#include "openhd-settings.hpp"
#include "openhd-status.hpp"
#include "openhd-telemetry.hpp"


#include "router.h"
#include "endpoint.h"
#include "tcpendpoint.h"
#include "udpendpoint.h"
#include "serialendpoint.h"


Router::Router(boost::asio::io_service &io_service, PlatformType platform_type, bool is_air, std::string unit_id, bool is_microservice):
    m_is_microservice(is_microservice),
    m_is_air(is_air),
    m_unit_id(unit_id),
    m_platform_type(platform_type),
    m_io_service(io_service) {

    configure();
}


void Router::configure() {
    std::cout << "Router::configure()" << std::endl;

    std::string platform_serial_port;

    switch (m_platform_type) {
        case PlatformTypeRaspberryPi: {
            platform_serial_port = "/dev/serial0";
            break;
        }
        case PlatformTypeJetson: {
            platform_serial_port = "/dev/ttyTHS1";
            break;
        }
        default: {
            // we default to using a USB serial adapter on any other platform at the moment, some just need
            // to be checked to see what the port is called, but PC will likely always be USB
            platform_serial_port = "/dev/ttyUSB0";
            break;
        }
    }

    /*
     * Use the most common settings (which happen to match the Microservice channel) by default, we will either
     * need to autodetect or expect users to switch it, which they will be able to do live
     */
    std::string platform_baud = "115200";
    m_tcp_port = 5760;
    m_telemetry_type = TelemetryTypeMavlink;

    std::vector<std::map<std::string, std::string> > settings;

    try {        
        // special handling for the microservice channel, which has no user settings
        if (m_is_microservice) {
            m_tcp_port = 5761;
            
            // this goes to the wifibroadcast channel and should never change
            m_udp_endpoints.push_back("15551:127.0.0.1:15550");

            // note: there are no serial connections on the microservice router, everything should be udp/tcp
            //       and it has no reason to be connecting to the flight controller or an antenna tracker
        } else {
            std::string settings_path = find_settings_path(m_is_air, m_unit_id);
            std::string settings_file = settings_path + "/telemetry.conf";
            std::cerr << "settings_file: " << settings_file << std::endl;
            settings = read_config(settings_file);

            std::string serial_ports;

            if (settings.size() > 0) {
                auto settings_map = settings.front();
                
                if (settings_map.count("telemetry_type")) {
                    m_telemetry_type = string_to_telemetry_type(settings_map["telemetry_type"]);
                }
                
                if (settings_map.count("serial_ports")) {
                    serial_ports = settings_map["serial_ports"];
                    if (serial_ports.size() > 0) {
                        std::vector<std::string> vec;
                        std::string _buf;
                        std::stringstream ss(serial_ports);

                        while (std::getline(ss, _buf, ',')) {
                            std::cout << _buf << std::endl;

                            boost::trim_right(_buf);
                            boost::trim_left(_buf);

                            m_serial_endpoints.push_back(_buf);
                        }
                    }
                } else {
                    m_serial_endpoints.push_back(platform_serial_port + ":" + platform_baud);
                }
            }

             // this goes to the wifibroadcast channel and should never change
            m_udp_endpoints.push_back("16551:127.0.0.1:16550");

            // goes to QOpenHD, hotspot devices should have their endpoint added dynamically in response to DHCP
            // or statically set in one of the settings files (not telemetry.conf, we should make it global so it
            // can be used to direct video to a specific device as well).
            m_udp_endpoints.push_back("14551:127.0.0.1:14550");
        }
    } catch (std::exception &ex) {
        std::cerr << "Telemetry settings load error: " << ex.what() << std::endl;
    }

    // save settings but only for the main telemetry service, we run a 2nd instance for the microservice channel
    // but it has no settings
    if (!m_is_microservice) {
        try {
            std::string settings_path = find_settings_path(m_is_air, m_unit_id);
            std::string settings_file = settings_path + "/telemetry.conf";

            save_settings(settings_file);
        } catch (std::exception &ex) {
            status_message(STATUS_LEVEL_EMERGENCY, "Telemetry settings save failed");
        }
    }

    m_tcp_acceptor = new boost::asio::ip::tcp::acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_tcp_port));


    for (auto & endpoint : m_udp_endpoints) {
        add_udp_endpoint(endpoint);
    }

    for (auto & endpoint : m_serial_endpoints) {
        add_serial_endpoint(endpoint);
    }

    start_accept();
}


void Router::add_udp_endpoint(std::string endpoint) {
    std::cout << "Adding UDP endpoint: " << endpoint << std::endl;

    UDPEndpoint::pointer new_connection = UDPEndpoint::create(this, m_io_service);
    std::static_pointer_cast<UDPEndpoint>(new_connection)->setup(m_telemetry_type, endpoint);
    m_endpoints.push_back(new_connection);
}


void Router::add_serial_endpoint(std::string endpoint) {
    std::cerr << "Adding Serial endpoint: " << endpoint << std::endl;

    SerialEndpoint::pointer new_connection = SerialEndpoint::create(this, m_io_service);
    std::static_pointer_cast<SerialEndpoint>(new_connection)->setup(m_telemetry_type, endpoint);
    m_endpoints.push_back(std::static_pointer_cast<SerialEndpoint>(new_connection));
}


void Router::command_received(std::string command_uri) {
    std::cerr << "Received command: " << command_uri << std::endl;

    boost::smatch result;

    boost::regex r{ "([\\w]+)\\:([\\w\\d\\.]+)\\:([\\d]+)|^list$"};
    if (!boost::regex_search(command_uri, result, r)) {
        std::cerr << "Failed to match regex" << std::endl;
        return;
    }

    if ((result.size() != 4) && result[1] != "list") {
        std::cerr << "Command format incorrect" << std::endl;

        return;
    }

    std::string command = result[1];

    if (command != "add" && command != "del" && command != "list") {
        std::cerr << "Invalid command received" << std::endl;

        return;
    }


    std::string address = result[2];
    std::string remote_port = result[3];

    std::stringstream _ep;


    if (command == "add") {
        std::vector<std::shared_ptr<Endpoint> >::iterator endpoint;
          // we don't know what port will be used yet so we use zero, the OS will pick one
        _ep << fmt::format("{}:{}:{}", 0, address, remote_port);

        for (endpoint = m_endpoints.begin(); endpoint != m_endpoints.end(); ) {
            std::cout << "*";
            if ((*endpoint)->get_medium() == "ser") 
                std::cout << "ser" << std::endl;
            else if ((*endpoint)->get_medium() == "tcp")
                std::cout << "tcp " << (boost::lexical_cast<std::string>(((*endpoint)->get_tcp_socket()).remote_endpoint())) << std::endl;
            else {
                std::cout << "udp " << (*endpoint)->get_address();
                if ((*endpoint)->get_address() == address) {
                    // already added this endpoint, skip it
                    std::cout << "already added" << std::endl;
                    return;
                }
            }
            ++endpoint;
        }

        // we don't know what port will be used yet so we use zero, the OS will pick one
        //_ep << fmt::format("{}:{}:{}", 0, address, remote_port);
        std::cout << "+"<< _ep.str() << std::endl;
        add_udp_endpoint(_ep.str());
    } else if (command == "del") {
        std::vector<std::shared_ptr<Endpoint> >::iterator endpoint;

         // we don't know what port will be used yet so we use zero, the OS will pick one
        _ep << fmt::format("{}:{}:{}", 0, address, remote_port);

        for (endpoint = m_endpoints.begin(); endpoint != m_endpoints.end();) {
            std::cout << "*";
            if ((*endpoint)->get_medium()=="udp") 
                std::cout << (*endpoint)->get_address() << std::endl;
            else if ((*endpoint)->get_medium()=="tcp") {
                std::cout << "tcp" << std::endl;
                //std::cout << (boost::lexical_cast<std::string>(((*endpoint)->get_tcp_socket()).remote_endpoint())) << std::endl;
            } if ((*endpoint)->get_address() == _ep.str()) {
                // we found it in the list, just remove it and that will stop any new packets from being sent to it
                std::cout << "-"<< (*endpoint)->get_address() << std::endl;
                m_endpoints.erase(endpoint);
                return;
            }
            ++endpoint;
        }
    } else if (command == "list") {
        for (auto endpoint = m_endpoints.begin(); endpoint != m_endpoints.end(); ++endpoint){
            std::cout << "*";
            if ((*endpoint)->get_medium()=="ser"){
                std::cout << "serial" << std::endl; 
            } else if ((*endpoint)->get_medium()=="udp"){
                std::cout << " UDP: " << (*endpoint)->get_address() << std::endl;
            } else {
                std::cout << " TCP: " << (boost::lexical_cast<std::string>(((*endpoint)->get_tcp_socket()).remote_endpoint())) << std::endl;
    }
        }
    }
}



void Router::save_settings(std::string settings_file) {
    Environment env;

    // load the telemetry template, we format it and write that to the file
    std::ifstream template_file("/usr/local/share/openhd/telemetry.template");
    std::string template_s((std::istreambuf_iterator<char>(template_file)),
                          std::istreambuf_iterator<char>());


    std::ofstream out(settings_file);

    json data;

    std::stringstream ss;
    for (size_t i = 0; i < m_serial_endpoints.size(); ++i) {
        if (i != 0) {
            ss << ",";
        }
        ss << m_serial_endpoints[i];
    }

    std::string serial_ports = ss.str();

    data["telemetry_type"] = telemetry_type_to_string(m_telemetry_type);
    data["serial_ports"] =  serial_ports;

    Template temp = env.parse(template_s.c_str());
    std::string rendered = env.render(temp, data);

    // and write it to the settings file
    out << rendered;
    out << "\n\n";
    
    out.close();
}


void Router::start_accept() {
    std::cerr << "Router::start_accept()" << std::endl;

    TCPEndpoint::pointer new_connection = TCPEndpoint::create(this, m_io_service);

    m_tcp_acceptor->async_accept(new_connection->get_tcp_socket(),
                                boost::bind(&Router::handle_accept,
                                            this,
                                            new_connection,
                                            boost::asio::placeholders::error));
}


void Router::handle_accept(TCPEndpoint::pointer new_connection, const boost::system::error_code& error) {
    std::cerr << "Router::handle_accept()" << std::endl;

    if (!error) {
        m_endpoints.push_back(new_connection);
        std::static_pointer_cast<TCPEndpoint>(new_connection)->setup(m_telemetry_type);
    }

    start_accept();
}


void Router::process_mavlink_message(Endpoint::pointer source_endpoint, mavlink_message_t msg) {
    std::cerr << "Router::process_mavlink_message()" << std::endl;

    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    auto size = mavlink_msg_to_send_buffer(buf, &msg);

    auto entry = mavlink_get_msg_entry(msg.msgid);


    int target_sys_id = -1;
    int target_comp_id = -1;

    if (entry) {
        if (entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM) {
            target_sys_id = (_MAV_PAYLOAD(&msg))[entry->target_system_ofs];
        }
        if (entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT) {
            target_comp_id = (_MAV_PAYLOAD(&msg))[entry->target_component_ofs];
        }
    }

    std::cerr << "Processing message " << msg.msgid << " from " << static_cast<int16_t>(msg.sysid) << ":" << static_cast<int16_t>(msg.compid) << " to " << static_cast<int16_t>(target_sys_id) << ":" << static_cast<int16_t>(target_comp_id) << std::endl;

    /*
     * This implements the routing logic described in https://ardupilot.org/dev/docs/mavlink-routing-in-ardupilot.html,
     * however we do not need to care about component IDs for routing purposes, only system IDs
     *
     */
    for (auto const& endpoint: m_endpoints) {
        auto send = false;

        if (target_sys_id == -1) {
            send = true;
        } else if (target_sys_id == 0) {
            send = true;
        } else {
            send = endpoint->seen_sys_id(target_sys_id);
        }

        // don't send the packet right back out the interface it came from
        if (source_endpoint != nullptr && source_endpoint == endpoint) {
            send = false;
        }

        if (send) {
            if (endpoint->get_medium() == "ser") {
                std::static_pointer_cast<SerialEndpoint>(endpoint)->send_message(buf, size);
            } else if (endpoint->get_medium() == "udp"){
                endpoint->send_message(buf, size);
            } else
                std::static_pointer_cast<TCPEndpoint>(endpoint)->send_message(buf, size);
        }
    }

//    std::cout << "----------------------------------------------------------------------------------------------------" << std::endl;
}


void Router::process_telemetry_message(Endpoint::pointer source_endpoint, uint8_t* buf, int size) {
    std::cerr << "Processing telemetry packet" << std::endl;

    /*
     * This just forwards the packet out of every endpoint except the one it arrived on
     *
     */
    for (auto const& endpoint: m_endpoints) {
        auto send = true;

        // don't send the packet right back out the interface it came from
        if (source_endpoint != nullptr && source_endpoint == endpoint) {
            send = false;
        }

        if (send) {
            if (endpoint->get_medium() == "ser"){
                std::static_pointer_cast<SerialEndpoint>(endpoint)->send_message(buf, size);
            } else {
                endpoint->send_message(buf, size);
            }
        }
    }
}


void Router::close_endpoint(std::shared_ptr<Endpoint> endpoint) {
    std::cerr << "Router::close_endpoint()" << std::endl;

    m_endpoints.erase(std::remove(m_endpoints.begin(), m_endpoints.end(), endpoint), m_endpoints.end());
    std::cerr << "Router::close_endpoint(): now have " << m_endpoints.size() << " endpoints " << std::endl;
}

