//
// Created by consti10 on 11.05.22.
//

#ifndef OPENHD_OHDTELEMETRY_H
#define OPENHD_OHDTELEMETRY_H

#include "AirTelemetry.h"
#include "GroundTelemetry.h"
#include <memory>
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

class OHDTelemetry{
public:
    OHDTelemetry(const OHDPlatform& platform,const OHDProfile& profile){
        if(profile.is_air){
            airTelemetry=std::make_unique<AirTelemetry>(AirTelemetry::uartForPlatformType(platform.platform_type));
        }else{
            groundTelemetry=std::make_unique<GroundTelemetry>();
        }
    }
    // only either one of them both is active at a time.
    // active when air
    std::unique_ptr<AirTelemetry> airTelemetry;
    // active when ground
    std::unique_ptr<GroundTelemetry> groundTelemetry;
};
#endif //OPENHD_OHDTELEMETRY_H
