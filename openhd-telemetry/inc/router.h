#ifndef ROUTER_H
#define ROUTER_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "openhd-platform.hpp"
#include "openhd-telemetry.hpp"

#include "endpoint.h"
#include "tcpendpoint.h"
#include "udpendpoint.h"

#include "serialendpoint.h"

class Router {
public:
    Router(boost::asio::io_service &io_service, PlatformType platform_type, bool is_air, std::string unit_id, bool is_microservice);
    void setup(std::vector<std::string> new_udp_endpoints);

    void start_accept();
    void handle_accept(TCPEndpoint::pointer new_connection, const boost::system::error_code& error);
    void close_endpoint(std::shared_ptr<Endpoint> endpoint);

    void handle_serial_read(char* buffer,
                            size_t size);

    void process_mavlink_message(Endpoint::pointer source_endpoint, mavlink_message_t msg);
    void process_telemetry_message(Endpoint::pointer source_endpoint, uint8_t* buf, int size);

    void add_known_sys_id(uint8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        if (!found) {
            std::cout << "Adding known sys ID for serial link: " << static_cast<int16_t>(sys_id) << std::endl;
            m_known_sys_ids.push_back(sys_id);
        }
    }

    bool seen_sys_id(uint8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        return found;
    }

private:
    void configure();
    
    void save_settings(std::string settings_file);

    void add_serial_endpoint(std::string endpoint);

    void add_udp_endpoint(std::string endpoint);

    bool m_is_microservice = false;

    bool m_is_air = false;
    std::string m_unit_id;

    int m_tcp_port = 5760;

    std::vector<std::string> m_udp_endpoints;
    std::vector<std::string> m_serial_endpoints;

    mavlink_status_t m_mavlink_status;

    PlatformType m_platform_type = PlatformTypeUnknown;
    TelemetryType m_telemetry_type = TelemetryTypeUnknown;

    std::vector<uint8_t> m_known_sys_ids;

    boost::asio::io_service &m_io_service;
    std::vector<std::shared_ptr<Endpoint> > m_endpoints;
    boost::asio::ip::tcp::acceptor *m_tcp_acceptor = nullptr;

    Serial *m_serial = nullptr;
};

#endif
