/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/
#ifndef OPENHD_AIR_TELEMETRY_PLUGIN_HOST_H
#define OPENHD_AIR_TELEMETRY_PLUGIN_HOST_H

#include <mutex>
#include <utility>
#include <vector>

#include "mav_include.h"
#include "plugins/air_telemetry_host.h"

namespace openhd::telemetry {

class AirTelemetryPluginHost {
 public:
  static AirTelemetryPluginHost &instance();

  const openhd_air_telemetry_host_interface *c_interface() const;

  void notify_rc_override(const mavlink_rc_channels_override_t &message);

 private:
  AirTelemetryPluginHost();
  ~AirTelemetryPluginHost();

  using Listener = std::pair<openhd_rc_channels_override_cb, void *>;

  void register_listener(openhd_rc_channels_override_cb callback,
                         void *user_data);
  void unregister_listener(openhd_rc_channels_override_cb callback,
                           void *user_data);

  static void register_listener_c(void *context, void *user_data,
                                  openhd_rc_channels_override_cb callback);
  static void unregister_listener_c(void *context, void *user_data,
                                    openhd_rc_channels_override_cb callback);

  mutable std::mutex mutex_;
  std::vector<Listener> listeners_;
  openhd_air_telemetry_host_interface interface_{};
};

}  // namespace openhd::telemetry

#endif  // OPENHD_AIR_TELEMETRY_PLUGIN_HOST_H
