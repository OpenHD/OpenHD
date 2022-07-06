//
// Created by consti10 on 11.05.22.
//

#ifndef OPENHD_OHDTELEMETRY_H
#define OPENHD_OHDTELEMETRY_H

#include "AirTelemetry.h"
#include "GroundTelemetry.h"
#include <memory>
#include <thread>
#include <utility>
#include "openhd-platform.hpp"
#include "openhd-profile.hpp"

/**
 * This class holds either a Air telemetry or Ground Telemetry instance.
 */
class OHDTelemetry {
 public:
  OHDTelemetry(OHDPlatform platform1,OHDProfile profile1,bool enableExtendedLogging=false) : platform(platform1),profile(std::move(profile1)),m_enableExtendedLogging(enableExtendedLogging) {
    if (this->profile.is_air) {
      airTelemetry = std::make_unique<AirTelemetry>(OHDTelemetry::uartForPlatformType(platform.platform_type));
      assert(airTelemetry);
      loopThread = std::make_unique<std::thread>([this] {
        assert(airTelemetry);
        airTelemetry->loopInfinite(this->m_enableExtendedLogging);
      });
    } else {
      groundTelemetry = std::make_unique<GroundTelemetry>();
      assert(groundTelemetry);
      loopThread = std::make_unique<std::thread>([this] {
        assert(groundTelemetry);
        groundTelemetry->loopInfinite(this->m_enableExtendedLogging);
      });
    }
  }
  // only either one of them both is active at a time.
  // active when air
  std::unique_ptr<AirTelemetry> airTelemetry;
  // active when ground
  std::unique_ptr<GroundTelemetry> groundTelemetry;
  std::unique_ptr<std::thread> loopThread;
  [[nodiscard]] std::string createDebug()const{
    if(profile.is_air){
      return airTelemetry->createDebug();
    }else{
      return groundTelemetry->createDebug();
    }
  }
  void add_settings_component(const int comp_id,std::shared_ptr<openhd::XSettingsComponent> glue){
    if(profile.is_air){
      airTelemetry->add_settings_component(comp_id,glue);
    }
  }
 private:
  const OHDPlatform platform;
  const OHDProfile profile;
  const bool m_enableExtendedLogging;
  /**
  * Return the name of the default UART for the different platforms OpenHD is running on.
  * @param platformType the platform we are running on
  * @return the uart name string (linux file)
   */
  static std::string uartForPlatformType(const PlatformType &platformType) {
    // we default to using a USB serial adapter on any other platform at the moment, some just need
    // to be checked to see what the port is called, but PC will likely always be USB
    // for testing, the serial shows up as this on my pc:
    std::string platformSerialPort = "/dev/ttyUSB0";
    switch (platformType) {
      case PlatformType::RaspberryPi: {
        platformSerialPort = "/dev/serial0";
        break;
      }
      case PlatformType::Jetson: {
        platformSerialPort = "/dev/ttyTHS1";
        break;
      }
      case PlatformType::PC:{
        platformSerialPort="/dev/ttyACM0";
        break;
      }
      default: {
        std::cout << "Using default UART " << platformSerialPort << "\n";
        break;
      }
    }
    return platformSerialPort;
  }
};
#endif //OPENHD_OHDTELEMETRY_H
