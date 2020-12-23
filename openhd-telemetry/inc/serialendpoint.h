#ifndef SERIALENDPOINT_H
#define SERIALENDPOINT_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"

class Router;


class SerialEndpoint: public Endpoint  {
public:
    static pointer create(Router *router, boost::asio::io_service& io_service) {
        return pointer(new SerialEndpoint(router, io_service));
    }

    void setup(TelemetryType telemetry_type) {};

    void setup(TelemetryType telemetry_type, std::string endpoint_s);

    void start_receive();

    void handle_serial_read(const boost::system::error_code& error,
                            size_t bytes_transferred);

    void handle_serial_write(const boost::system::error_code& error,
                             size_t bytes_transferred);

    void send_message(uint8_t* buf, size_t size);

protected:    
    boost::asio::serial_port m_serial;

private:
    SerialEndpoint(Router *router, boost::asio::io_service& io_service): Endpoint(router, io_service), m_serial(io_service) {}
};

#endif
