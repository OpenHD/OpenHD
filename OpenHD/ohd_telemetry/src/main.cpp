#include <iostream>

#include "GroundTelemetry.h"
#include "AirTelemetry.h"
#include "openhd-profile.hpp"

static constexpr auto TAG="XMAVLINK_SERVICE";
int main() {
    std::cout <<TAG<< "start\n";

    const auto profile=profile_from_manifest();

   const bool AIR=profile.is_air;
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
