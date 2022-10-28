//
// Created by consti10 on 13.08.22.
//

#include "OHDTelemetry.h"

#include "AirTelemetry.h"
#include "GroundTelemetry.h"

OHDTelemetry::OHDTelemetry(OHDPlatform platform1,
						   OHDProfile profile1,
						   std::shared_ptr<openhd::ActionHandler> action_handler,
						   bool enableExtendedLogging) :
	platform(platform1),profile(std::move(profile1)),m_enableExtendedLogging(enableExtendedLogging) {
  if (this->profile.is_air) {
	airTelemetry = std::make_unique<AirTelemetry>(platform,action_handler);
	assert(airTelemetry);
	loopThread = std::make_unique<std::thread>([this] {
	  assert(airTelemetry);
	  airTelemetry->loopInfinite(this->m_enableExtendedLogging);
	});
  } else {
	groundTelemetry = std::make_unique<GroundTelemetry>(platform,action_handler);
	assert(groundTelemetry);
	loopThread = std::make_unique<std::thread>([this] {
	  assert(groundTelemetry);
	  groundTelemetry->loopInfinite(this->m_enableExtendedLogging);
	});
  }
}

std::string OHDTelemetry::createDebug() const {
  if(profile.is_air){
	return airTelemetry->createDebug();
  }else{
	return groundTelemetry->createDebug();
  }
}

void OHDTelemetry::add_settings_generic(const std::vector<openhd::Setting> &settings) const {
  if(profile.is_air){
	airTelemetry->add_settings_generic(settings);
  }else{
	groundTelemetry->add_settings_generic(settings);
  }
}
void OHDTelemetry::settings_generic_ready() const {
  if(profile.is_air){
	airTelemetry->settings_generic_ready();
  }else{
	groundTelemetry->settings_generic_ready();
  }
}
void OHDTelemetry::add_camera_component(const int camera_index, const std::vector<openhd::Setting> &settings) const {
  // we only have cameras on the air telemetry unit
  assert(profile.is_air);
  // only 2 cameras suported for now.
  airTelemetry->add_camera_component(camera_index,settings);
}

void OHDTelemetry::set_link_statistics(openhd::link_statistics::AllStats stats) const {
  if(profile.is_air){
	airTelemetry->set_link_statistics(stats);
  }else{
	groundTelemetry->set_link_statistics(stats);
  }
}

void OHDTelemetry::add_external_ground_station_ip(const std::string &ip_openhd, const std::string &ip_dest_device) const {
  // We only support external ground station(s) connecting to the ground unit
  if(profile.is_air)return;
  groundTelemetry->add_external_ground_station_ip(ip_openhd,ip_dest_device);
}

void OHDTelemetry::remove_external_ground_station_ip(const std::string &ip_openhd,const std::string &ip_dest_device) const {
  // We only support external ground station(s) connecting to the ground unit
  if(profile.is_air)return;
  groundTelemetry->remove_external_ground_station_ip(ip_openhd,ip_dest_device);
}
