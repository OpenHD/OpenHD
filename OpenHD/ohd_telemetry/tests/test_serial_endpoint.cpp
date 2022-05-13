//
// Created by consti10 on 19.04.22.
// Executable to test the serial endpoint implementation
// For testing, just connect a flight controller via uart and look at the console logs
//

#include <iostream>

#include "../src/endpoints/SerialEndpoint.h"
#include "../src/mav_helper.h"

int main() {
    std::cout<< "SerialEndpointTest::start" << std::endl;
    SerialEndpoint::HWOptions opt{SerialEndpoint::TEST_SERIAL_PORT};
    SerialEndpoint serialEndpoint("TestSerialPort",opt);
    serialEndpoint.registerCallback([](MavlinkMessage& msg){
        debugMavlinkMessage(msg.m,"SerialTest");
    });
    // now mavlink messages should come in. Try disconnecting and reconnecting, and see if messages continue
    const auto start=std::chrono::steady_clock::now();
    while ((std::chrono::steady_clock::now()-start)<std::chrono::minutes(5)){
        serialEndpoint.debugIfAlive();
        auto msg=MExampleMessage::heartbeat();
        serialEndpoint.sendMessage(msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout<< "SerialEndpointTest::end" << std::endl;
    return 0;
}


