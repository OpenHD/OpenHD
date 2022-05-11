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
            airTelemetry=std::make_unique<AirTelemetry>(OHDTelemetry::uartForPlatformType(platform.platform_type));
        }else{
            groundTelemetry=std::make_unique<GroundTelemetry>();
        }
    }
    // only either one of them both is active at a time.
    // active when air
    std::unique_ptr<AirTelemetry> airTelemetry;
    // active when ground
    std::unique_ptr<GroundTelemetry> groundTelemetry;
private:
    /**
    * Return the name of the default UART for the different platforms OpenHD is running on.
    * @param platformType the platform we are running on
    * @return the uart name string (linux file)
    */
    static std::string uartForPlatformType(const PlatformType& platformType){
        // we default to using a USB serial adapter on any other platform at the moment, some just need
        // to be checked to see what the port is called, but PC will likely always be USB
        std::string platformSerialPort="/dev/ttyUSB0";
        switch (platformType) {
            case PlatformTypeRaspberryPi: {
                platformSerialPort = "/dev/serial0";
                break;
            }
            case PlatformTypeJetson: {
                platformSerialPort = "/dev/ttyTHS1";
                break;
            }
            default: {
                std::cerr<<"Using default UART "<<platformSerialPort<<"\n";
                break;
            }
        }
        return platformSerialPort;
    }
};
#endif //OPENHD_OHDTELEMETRY_H
