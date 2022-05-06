//
// Created by consti10 on 24.04.22.
// Test the OHD telemetry generation - this just prints the generated data to stdout.
//
#include "../src/ohd_telemetry/InternalTelemetry.h"
#include "../src/ohd_telemetry/LogCustomOHDMessages.hpp"
#include <iostream>
#include <thread>

int main() {
    std::cout<< "OHDTelemetryGeneratorTest::start" << std::endl;

    InternalTelemetry ohdTelemetryGenerator(false);
    const auto start=std::chrono::steady_clock::now();
    while ((std::chrono::steady_clock::now()-start)<std::chrono::minutes(5)){
        const auto msges=ohdTelemetryGenerator.generateUpdates();
        LogCustomOHDMessages::logMessages(msges);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout<< "OHDTelemetryGeneratorTest::end" << std::endl;
    return 0;
}
