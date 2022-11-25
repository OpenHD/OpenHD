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
#include "mavlink_settings/ISettingsComponent.hpp"
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
			   std::shared_ptr<openhd::ActionHandler> opt_action_handler=nullptr,
			   bool enableExtendedLogging=false);
  OHDTelemetry(const OHDTelemetry&)=delete;
  OHDTelemetry(const OHDTelemetry&&)=delete;
  ~OHDTelemetry();
  [[nodiscard]] std::string createDebug()const;
  // Settings and statistics. Other modules (e.g. video, interface) use the mavlink settings
  // provided by OHD Telemetry. However, we do not have code dependencies directly between these modules,
  // to allow independent testing without telemetry and to keep the functionalities seperated.
  // All modules other than camera share the same settings component for now.
  // Note that the settings are still experiencing changes / are not finalized, e.g. we might introduce
  // different settings components for different OHD modules if viable.
  void add_settings_generic(const std::vector<openhd::Setting>& settings) const;
  // This is confusing, but there is no way around (keyword: invariant settings), since we add the settings one at a time as we create
  // the other modules (e.g. interface, video) sequentially one at a time in the OHD main.cpp file.
  // Note that without calling this function, no ground station will see any settings, even though they are already added.
  void settings_generic_ready() const;
  // Cameras get their own component ID, other than the "rest" which shares the same component id
  // for simplicity. Note, at some point it might make sense to also use its own component id
  // for OHD interface
  void add_camera_component(int camera_index,const std::vector<openhd::Setting>& settings) const;
  // Add / remove the IP of another Ground station client. Buggy / not finished yet.
  void add_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device)const;
  void remove_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device)const;
 private:
  // only either one of them both is active at a time.
  // active when air
  std::unique_ptr<AirTelemetry> airTelemetry;
  // active when ground
  std::unique_ptr<GroundTelemetry> groundTelemetry;
  // Main telemetry thread. Note that the endpoints also might have their own
  // Receive threads
  std::unique_ptr<std::thread> loopThread;
  const OHDPlatform platform;
  const OHDProfile profile;
  const bool m_enableExtendedLogging;
  bool terminate=false;
};

#endif //OPENHD_OHDTELEMETRY_H
