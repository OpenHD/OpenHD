#include <iostream>

#include "GroundTelemetry.h"
#include "AirTelemetry.h"
#include <thread>
// OpenHD stuff
#include "json.hpp"
using json = nlohmann::json;
#include "openhd-platform.hpp"
#include "openhd-settings.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"

static constexpr auto TAG="XMAVLINK_SERVICE";
int main() {
    std::cout <<TAG<< "start\n";

   const bool AIR=runs_on_air();
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
