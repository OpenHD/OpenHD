//
// Created by consti10 on 11.05.22.
//

#ifndef OPENHD_OHDTELEMETRY_H
#define OPENHD_OHDTELEMETRY_H

#include "AirTelemetry.h"
#include "GroundTelemetry.h"

#include "openhd-platform.hpp"
#include "openhd-profile.hpp"
#include "openhd-action-handler.hpp"
#include "mavlink_settings/ISettingsComponent.h"
#include "openhd-link-statistics.hpp"
#include <memory>
#include <thread>
#include <utility>

// Forward declare them to speed up compilation time.
//class AirTelemetry;
//class GroundTelemetry;

/**
 * This class holds either a Air telemetry or Ground Telemetry instance.
 */
class OHDTelemetry {
 public:
  OHDTelemetry(OHDPlatform platform1,OHDProfile profile1,
			   std::shared_ptr<openhd::ActionHandler> action_handler=nullptr,
			   bool enableExtendedLogging=false);
  OHDTelemetry(const OHDTelemetry&)=delete;
  OHDTelemetry(const OHDTelemetry&&)=delete;
  // only either one of them both is active at a time.
  // active when air
  std::unique_ptr<AirTelemetry> airTelemetry;
  // active when ground
  std::unique_ptr<GroundTelemetry> groundTelemetry;
  std::unique_ptr<std::thread> loopThread;
  [[nodiscard]] std::string createDebug()const;
  // All modules other than camera share the same settings component for now.
  void add_settings_generic(const std::vector<openhd::Setting>& settings) const;
  void settings_generic_ready() const;
  // Cameras get their own component ID, other than the "rest" which shares the same component id
  // for simplicity. Note, at some point it might make sense to also use its own component id
  // for OHD interface
  void add_camera_component(int camera_index,const std::vector<openhd::Setting>& settings) const;
  void set_link_statistics(openhd::link_statistics::AllStats stats) const;
  // Add the IP of another Ground station client
  void add_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device)const;
  // Add the IP of another Ground station client
  void remove_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device)const;
 private:
  const OHDPlatform platform;
  const OHDProfile profile;
  const bool m_enableExtendedLogging;
};

#endif //OPENHD_OHDTELEMETRY_H
