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
#include "plugins/plugins.h"

#include "RaspberryPiRoverMotors.h"
#include "openhd_spdlog.h"
#include "plugins/air_telemetry_host.h"

#include <cmath>
#include <cstdint>
#include <memory>

#include <spdlog/spdlog.h>

namespace {

constexpr uint16_t kMavlinkChannelUnset = UINT16_MAX;
constexpr int kNeutralPwm = 1500;
constexpr int kTurnRightThreshold = 1600;
constexpr int kTurnLeftThreshold = 1400;

struct MotorPluginContext {
  std::shared_ptr<spdlog::logger> logger;
  std::unique_ptr<RaspberryPiRoverMotors> motors;
  const openhd_air_telemetry_host_interface *host_interface = nullptr;
  bool callback_registered = false;
};

int decode_channel(uint16_t raw_value, int fallback) {
  return raw_value == kMavlinkChannelUnset ? fallback
                                           : static_cast<int>(raw_value);
}

void handle_rc_override(const openhd_rc_channels_override *override_data,
                        void *user_data) {
  if (!override_data || !user_data) {
    return;
  }
  auto *context = static_cast<MotorPluginContext *>(user_data);
  if (!context->motors || !context->motors->initialized()) {
    return;
  }
  if (override_data->channel_count < 4) {
    return;
  }

  const int raw_x = decode_channel(override_data->channels[0], kNeutralPwm);
  const int raw_y = decode_channel(override_data->channels[3], kNeutralPwm);

  const int mapped_speed = context->motors->map_speed(raw_y - kNeutralPwm);
  const int pwm_value = std::abs(mapped_speed);

  if (pwm_value == 0) {
    context->motors->stop();
    return;
  }

  context->motors->set_speed(pwm_value);

  if (raw_x > kTurnRightThreshold) {
    context->motors->set_direction_motor_a(true);
    context->motors->set_direction_motor_b(false);
  } else if (raw_x < kTurnLeftThreshold) {
    context->motors->set_direction_motor_a(false);
    context->motors->set_direction_motor_b(true);
  } else {
    context->motors->stop();
  }
}

}  // namespace

extern "C" {

struct openhd_plugin_context {
  MotorPluginContext impl;
};

const struct openhd_plugin_info *openhd_plugin_get_info() {
  static const struct openhd_plugin_info kInfo = {
      OPENHD_PLUGIN_API_VERSION,
      "rpi_rover_motors",
      "Drives differential rover motors using MAVLink RC override messages.",
      openhd_plugin_type::UNKNOWN,
      0,
  };
  return &kInfo;
}

struct openhd_plugin_context *openhd_plugin_init(void *host_context) {
  auto *context = new openhd_plugin_context();
  context->impl.logger = openhd::log::create_or_get("rpi_rover_motors");
  if (context->impl.logger) {
    context->impl.logger->set_level(spdlog::level::info);
    context->impl.logger->info("Initializing Raspberry Pi rover motors plugin");
  }

  context->impl.motors = std::make_unique<RaspberryPiRoverMotors>(22, 27, 23, 24, 18);
  if (!context->impl.motors || !context->impl.motors->initialized()) {
    if (context->impl.logger) {
      context->impl.logger->error("Failed to initialize pigpio – disabling plugin");
    }
    delete context;
    return nullptr;
  }

  const auto *host = static_cast<const openhd_plugin_host_context *>(host_context);
  if (host && host->air_telemetry) {
    context->impl.host_interface = host->air_telemetry;
    context->impl.host_interface->register_rc_override_callback(
        context->impl.host_interface->context, &context->impl, handle_rc_override);
    context->impl.callback_registered = true;
    if (context->impl.logger) {
      context->impl.logger->info("Registered RC override listener");
    }
  } else if (context->impl.logger) {
    context->impl.logger->warn(
        "Air telemetry host interface unavailable – motor control disabled");
  }

  return context;
}

void openhd_plugin_shutdown(struct openhd_plugin_context *context) {
  if (!context) {
    return;
  }
  if (context->impl.callback_registered && context->impl.host_interface) {
    context->impl.host_interface->unregister_rc_override_callback(
        context->impl.host_interface->context, &context->impl, handle_rc_override);
  }
  if (context->impl.logger) {
    context->impl.logger->info("Shutting down Raspberry Pi rover motors plugin");
  }
  context->impl.motors.reset();
  delete context;
}

}  // extern "C"
