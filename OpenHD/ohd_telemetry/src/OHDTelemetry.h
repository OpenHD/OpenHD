//
// Created by consti10 on 11.05.22.
//

#ifndef OPENHD_OHDTELEMETRY_H
#define OPENHD_OHDTELEMETRY_H

#include <memory>
#include <thread>
#include <utility>

#include "openhd_action_handler.h"
#include "openhd_external_device.h"
#include "openhd_link.hpp"
#include "openhd_link_statistics.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_settings_imp.h"

// Forward declare them to speed up compilation time.
class AirTelemetry;
class GroundTelemetry;

/**
 * This class holds either a Air telemetry or Ground Telemetry instance.
 */
class OHDTelemetry {
 public:
  OHDTelemetry(OHDProfile profile1, bool enableExtendedLogging = false);
  OHDTelemetry(const OHDTelemetry&) = delete;
  OHDTelemetry(const OHDTelemetry&&) = delete;
  ~OHDTelemetry();
  [[nodiscard]] std::string createDebug() const;
  // Settings and statistics. Other modules (e.g. video, interface) use the
  // mavlink settings provided by OHD Telemetry. However, we do not have code
  // dependencies directly between these modules, to allow independent testing
  // without telemetry and to keep the functionalities seperated. All modules
  // other than camera share the same settings component for now. Note that the
  // settings are still experiencing changes / are not finalized, e.g. we might
  // introduce different settings components for different OHD modules if
  // viable.
  void add_settings_generic(const std::vector<openhd::Setting>& settings) const;
  // This is confusing, but there is no way around (keyword: invariant
  // settings), since we add the settings one at a time as we create the other
  // modules (e.g. interface, video) sequentially one at a time in the OHD
  // main.cpp file. Note that without calling this function, no ground station
  // will see any settings, even though they are already added.
  void settings_generic_ready() const;
  // Cameras get their own component ID, other than the "rest" which shares the
  // same component id for simplicity. Note, at some point it might make sense
  // to also use its own component id for OHD interface
  void add_settings_camera_component(
      int camera_index, const std::vector<openhd::Setting>& settings) const;
  // OHDTelemetry is agnostic of the type of transmission between air and ground
  // and also agnostic weather this link exists or not (since it is already
  // using a lossy link).
  void set_link_handle(std::shared_ptr<OHDLink> link);

 private:
  // only either one of them both is active at a time.
  // active when air
  std::unique_ptr<AirTelemetry> m_air_telemetry;
  // active when ground
  std::unique_ptr<GroundTelemetry> m_ground_telemetry;
  // Main telemetry thread. Note that the endpoints also might have their own
  // Receive threads
  std::unique_ptr<std::thread> m_loop_thread;
  bool m_loop_thread_terminate = false;
  const OHDProfile m_profile;
  const bool m_enableExtendedLogging;
};

#endif  // OPENHD_OHDTELEMETRY_H
