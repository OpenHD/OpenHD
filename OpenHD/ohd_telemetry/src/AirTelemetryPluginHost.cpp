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
#include "AirTelemetryPluginHost.h"

#include <algorithm>
#include <cstring>

#include "mav_include.h"

namespace openhd::telemetry {
namespace {
constexpr size_t kRcChannelCount = 8;
}

AirTelemetryPluginHost &AirTelemetryPluginHost::instance() {
  static AirTelemetryPluginHost host;
  return host;
}

AirTelemetryPluginHost::AirTelemetryPluginHost() {
  interface_.context = this;
  interface_.register_rc_override_callback = &AirTelemetryPluginHost::register_listener_c;
  interface_.unregister_rc_override_callback = &AirTelemetryPluginHost::unregister_listener_c;
}

AirTelemetryPluginHost::~AirTelemetryPluginHost() = default;

const openhd_air_telemetry_host_interface *
AirTelemetryPluginHost::c_interface() const {
  return &interface_;
}

void AirTelemetryPluginHost::register_listener(
    openhd_rc_channels_override_cb callback, void *user_data) {
  if (!callback) {
    return;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  listeners_.emplace_back(callback, user_data);
}

void AirTelemetryPluginHost::unregister_listener(
    openhd_rc_channels_override_cb callback, void *user_data) {
  std::lock_guard<std::mutex> lock(mutex_);
  listeners_.erase(std::remove_if(listeners_.begin(), listeners_.end(),
                                  [callback, user_data](const Listener &entry) {
                                    return entry.first == callback &&
                                           entry.second == user_data;
                                  }),
                   listeners_.end());
}

void AirTelemetryPluginHost::notify_rc_override(
    const mavlink_rc_channels_override_t &message) {
  openhd_rc_channels_override payload{};
  payload.channel_count = kRcChannelCount;
  uint16_t raw[kRcChannelCount] = {message.chan1_raw, message.chan2_raw,
                                   message.chan3_raw, message.chan4_raw,
                                   message.chan5_raw, message.chan6_raw,
                                   message.chan7_raw, message.chan8_raw};
  std::memcpy(payload.channels, raw, sizeof(raw));

  std::vector<Listener> listeners_copy;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_copy = listeners_;
  }

  for (const auto &listener : listeners_copy) {
    if (listener.first) {
      listener.first(&payload, listener.second);
    }
  }
}

void AirTelemetryPluginHost::register_listener_c(
    void *context, void *user_data, openhd_rc_channels_override_cb callback) {
  if (!context) {
    return;
  }
  auto *host = static_cast<AirTelemetryPluginHost *>(context);
  host->register_listener(callback, user_data);
}

void AirTelemetryPluginHost::unregister_listener_c(
    void *context, void *user_data, openhd_rc_channels_override_cb callback) {
  if (!context) {
    return;
  }
  auto *host = static_cast<AirTelemetryPluginHost *>(context);
  host->unregister_listener(callback, user_data);
}

}  // namespace openhd::telemetry
