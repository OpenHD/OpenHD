//
// Created by consti10 on 27.04.22.
// For testing, run the air and ground telemetry services side by side on the same machine locally.
//
#include <iostream>

#include "../src/GroundTelemetry.h"
#include "../src/AirTelemetry.h"
#include "../src/OHDTelemetry.hpp"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include <thread>
#include <memory>

static constexpr auto TAG="XMAVLINK_SERVICE_TEST";
int main() {
    std::cout << TAG << "start\n";
    // Start one service in its own thread
    std::thread air([]{
        AirTelemetry airTelemetry{SerialEndpoint::TEST_SERIAL_PORT};
        airTelemetry.loopInfinite();
    });
    // And run the other one, which blocks until error.
    GroundTelemetry groundTelemetry{};
    groundTelemetry.loopInfinite();
    std::cout << TAG << "end\n";
    /*std::unique_ptr<OHDTelemetry> air;
    std::unique_ptr<OHDTelemetry> ground;
    {
        OHDProfile profile{true,"XX"};
        OHDPlatform platform{};
        air=std::make_unique<OHDTelemetry>(platform,profile);
    }
    {
        OHDProfile profile{false,"XX"};
        OHDPlatform platform{};
        ground=std::make_unique<OHDTelemetry>(platform,profile);
    }
    while (true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if(air && ground){
            std::cout<<"A & G\n";
        }
    }*/
}

