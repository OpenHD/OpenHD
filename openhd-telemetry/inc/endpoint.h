#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "openhd-telemetry.hpp"

class Router;

class Endpoint: public std::enable_shared_from_this<Endpoint> {
public:
    typedef std::shared_ptr<Endpoint> pointer;

    virtual void setup(TelemetryType telemetry_type) = 0;
    virtual void setup(TelemetryType telemetry_type, std::string endpoint_s) = 0;

    void set_telemetry_type(TelemetryType telemetry_type) {
        m_telemetry_type = telemetry_type;
    }

    void add_known_sys_id(uint8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        if (!found) {
            std::cout << "Adding known sys ID for endpoint: " << static_cast<int16_t>(sys_id) << std::endl;
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



    boost::asio::ip::tcp::socket& get_tcp_socket() {
        return m_tcp_socket;
    }

    boost::asio::ip::udp::socket& get_udp_socket() {
        return m_udp_socket;
    }

    std::string get_address() {
        return m_address;
    }

    virtual void send_message(uint8_t *buf, int size);

protected:
    Router *m_router = nullptr;

    std::vector<uint8_t> m_known_sys_ids;

    std::string m_address;

    TelemetryType m_telemetry_type = TelemetryTypeUnknown;

    enum { max_length = 1024 };
    char data[max_length];

    mavlink_status_t m_mavlink_status;

    boost::asio::ip::udp::socket m_udp_socket;
    boost::asio::ip::tcp::socket m_tcp_socket;

    Endpoint(Router *router, boost::asio::io_service& io_service): m_router(router), m_udp_socket(io_service), m_tcp_socket(io_service) {}
};

#endif // ENDPOINT_H
