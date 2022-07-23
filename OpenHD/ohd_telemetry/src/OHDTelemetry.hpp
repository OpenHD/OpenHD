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
      airTelemetry = std::make_unique<AirTelemetry>(platform,OHDTelemetry::uartForPlatformType(platform.platform_type));
      assert(airTelemetry);
      loopThread = std::make_unique<std::thread>([this] {
        assert(airTelemetry);
        airTelemetry->loopInfinite(this->m_enableExtendedLogging);
      });
    } else {
      groundTelemetry = std::make_unique<GroundTelemetry>(platform);
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
  void add_settings_component(const int comp_id,std::shared_ptr<openhd::ISettingsComponent> glue){
	assert(_already_added_settings_components.find(comp_id)==_already_added_settings_components.end());
	_already_added_settings_components[comp_id]=nullptr;
    if(profile.is_air){
      airTelemetry->add_settings_component(comp_id,std::move(glue));
    }else{
      groundTelemetry->add_settings_component(comp_id,std::move(glue));
    }
  }
  void add_camera_component(const int camera_index,std::shared_ptr<openhd::ISettingsComponent> glue){
    // we only have cameras on the air telemetry unit
    assert(profile.is_air);
    // only 2 cameras suported for now.
    assert(camera_index>=0 && camera_index<2);
    airTelemetry->add_settings_component(MAV_COMP_ID_CAMERA+camera_index,std::move(glue));
  }
  void set_link_statistics(openhd::link_statistics::AllStats stats) const{
	if(profile.is_air){
	  airTelemetry->set_link_statistics(stats);
	}else{
	  groundTelemetry->set_link_statistics(stats);
	}
  }
  // Add the IP of another Ground station client
  void add_external_ground_station_ip(std::string ip_openhd,std::string ip_dest_device)const{
	assert(!profile.is_air);
	groundTelemetry->add_external_ground_station_ip(std::move(ip_openhd),std::move(ip_dest_device));
  }
 private:
  const OHDPlatform platform;
  const OHDProfile profile;
  const bool m_enableExtendedLogging;
  std::map<uint8_t,void*> _already_added_settings_components;
  /**
  * Return the name of the default UART for the different platforms OpenHD is running on.
  * @param platformType the platform we are running on
  * @return the uart name string (linux file)
   */
  static std::string uartForPlatformType(const PlatformType &platformType) {
    // hacky for now, this works on rpi when connecting the USB of my FC
    return "/dev/ttyACM0";

    // we default to using a USB serial adapter on any other platform at the moment, some just need
    // to be checked to see what the port is called, but PC will likely always be USB
    // for testing, the serial shows up as this on my pc:
    /*std::string platformSerialPort = "/dev/ttyUSB0";
    switch (platformType) {
      case PlatformType::RaspberryPi: {
        //platformSerialPort = "/dev/serial0";
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
    return platformSerialPort;*/
  }
};

#endif //OPENHD_OHDTELEMETRY_H
