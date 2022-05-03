#include <iostream>

#include "GroundTelemetry.h"
#include "AirTelemetry.h"
// OpenHD stuff
#include "openhd-read-util.hpp"

static constexpr auto TAG="XMAVLINK_SERVICE";
int main() {
    std::cout <<TAG<< "start\n";

   const bool AIR=OHDReadUtil::runs_on_air();
   std::cout<<"Starting "<<TAG<<" air:"<<(AIR ? "Y":"N");
   if(AIR){
       AirTelemetry airTelemetry{};
       airTelemetry.loopInfinite();
   }else{
       GroundTelemetry groundTelemetry{};
       groundTelemetry.loopInfinite();
   }
    return 0;
}
