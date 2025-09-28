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
#ifndef OPENHD_PLUGINS_AIR_TELEMETRY_HOST_H
#define OPENHD_PLUGINS_AIR_TELEMETRY_HOST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct openhd_rc_channels_override {
  uint16_t channels[8];
  size_t channel_count;
};

typedef void (*openhd_rc_channels_override_cb)(
    const struct openhd_rc_channels_override *override_data,
    void *user_data);

struct openhd_air_telemetry_host_interface {
  void *context;
  void (*register_rc_override_callback)(
      void *context, void *user_data,
      openhd_rc_channels_override_cb callback);
  void (*unregister_rc_override_callback)(
      void *context, void *user_data,
      openhd_rc_channels_override_cb callback);
};

struct openhd_plugin_host_context {
  const struct openhd_air_telemetry_host_interface *air_telemetry;
};

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGINS_AIR_TELEMETRY_HOST_H
