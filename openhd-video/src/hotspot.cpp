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

#include <json.hpp>

#include <fmt/core.h>

#include "openhd-platform.hpp"
#include "openhd-status.hpp"
#include "openhd-video.hpp"

#include "hotspot.h"

int sendMeasuredSpeed = 500;

/* this is the point where video is actually received on the ground side directly from wfb_rx, this class
 * distributes it around to the Record class and to user devices.
 * 
 * It could just as easily be called Distribution or something similar, but its primary purpose is hotspot
 * routing.
 */

Hotspot::Hotspot(boost::asio::io_service &io_service, uint16_t port): 
    m_io_service(io_service),
    m_port(port),
    m_udp_socket(io_service) {
    std::cerr << "Hotspot::Hotspot()" << port << std::endl;

    setup();
}


void Hotspot::setup() {
    std::cerr << "Hotspot::setup()" << std::endl;

    // goes to QOpenHD, hotspot devices should have their endpoint added dynamically in response to DHCP
    // or statically set in one of the settings files
    m_endpoints.push_back("127.0.0.1");

    m_udp_socket.open(boost::asio::ip::udp::v4());
    m_udp_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), m_port));
    
    start_receive();
}


void Hotspot::start_receive() {
    m_udp_socket.async_receive(boost::asio::buffer(data, max_length),
                               boost::bind(&Hotspot::handle_receive, 
                                           this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
}


void Hotspot::handle_receive(const boost::system::error_code& error,
                             size_t bytes_transferred) {
    if (!error) {        
        // mirror the video back to the recording system
        auto _record_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), m_port - 10);
        
        m_udp_socket.send_to(boost::asio::buffer(data, bytes_transferred), _record_endpoint);

        for (auto & endpoint : m_endpoints) {            
            auto _endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(endpoint), m_port - 20);
        
            m_udp_socket.send_to(boost::asio::buffer(data, bytes_transferred), _endpoint);
        }
        
        if (sendMeasuredSpeed--<0){
            std::string bitRate;
            const uint64_t runTime=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-INIT_TIME).count();
            INIT_TIME = std::chrono::steady_clock::now();
            uint16_t br = (tot_bytes_transferred/runTime)*8;
            std::cout << "VS:" << +(br/1000) << "Mb/s" << std::endl;
            
            std::string bitRate = fmt::format ("vbr{0:d}end",br);
            tot_bytes_transferred = 0;
            sendMeasuredSpeed = 500;
            auto _status_video_bitrate = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 50000);
            m_udp_socket.send_to(boost::asio::buffer(bitRate, bitRate.size()), _status_video_bitrate);
        }
        tot_bytes_transferred+= bytes_transferred;

        start_receive();
    }
}



void Hotspot::command_received(std::string command_uri) {
    std::cerr << "Received command: " << command_uri << std::endl;

    boost::smatch result;

    boost::regex r{ "([\\w]+)\\:([\\w\\d\\.]+)"};
    if (!boost::regex_search(command_uri, result, r)) {
        std::cerr << "Failed to match regex" << std::endl;
        return;
    }

    if (result.size() != 3) {
        std::cerr << "Command format incorrect" << std::endl;

        return;
    }

    std::string command = result[1];

    if (command != "add" && command != "del" && command != "list") {
        std::cerr << "Invalid command received" << std::endl;

        return;
    }

    std::string address = result[2];

    if (command == "add") {
        for (auto & endpoint : m_endpoints) {
        std::cerr << "*"<< endpoint << std::endl;
            if (endpoint == address) {
                // already added this endpoint, skip it
                return;
            }
        }

        m_endpoints.push_back(address);
        std::cerr << "New endpoint added" << address << std::endl;

    } else if (command == "del") {
        auto ep = m_endpoints.begin();
        while (ep != m_endpoints.end()){
            std::cerr << "*"<< *ep << std::endl;
            if (*ep == address) {
                std::cerr << "-"<< *ep << std::endl;
                m_endpoints.erase(ep);
                //return;
            }
            else
            {
                ep++;
            }
        }

    } else if (command == "list") {
        for (auto const & endpoint : m_endpoints) {
            std::cerr << "*"<< endpoint << ":" << m_port << std::endl;
        }
    }
}

