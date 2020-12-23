#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <openhd/mavlink.h>

#include "router.h"
#include "endpoint.h"

void Endpoint::setup(TelemetryType telemetry_type) {}
void Endpoint::setup(TelemetryType telemetry_type, std::string endpoint_s) {}

void Endpoint::send_message(uint8_t *buf, int size) {}
