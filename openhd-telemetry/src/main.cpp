#include <iostream>

#include "GroundTelemetry.h"
#include "AirTelemetry.h"
#include <thread>

static constexpr auto TAG="XMAVLINK_SERVICE";
int main() {
    std::cout <<TAG<< "start\n";

    /*const bool AIR= false;
    if(AIR){
        AirTelemetry airTelemetry{};
        airTelemetry.loopInfinite();
    }else{
        GroundTelemetry groundTelemetry{};
        groundTelemetry.loopInfinite();
    }*/
    std::thread air([]{
        AirTelemetry airTelemetry{};
        airTelemetry.loopInfinite();
    });
    //
    GroundTelemetry groundTelemetry{};
    groundTelemetry.loopInfinite();

    return 0;
}
