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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_

#include "../routing/MavlinkComponent.hpp"
#include "sender.h"

namespace mavsdk {

class SenderWrapper : public Sender {
 public:
  explicit SenderWrapper(MavlinkComponent& comp) : _mavlink_component(comp) {}
  MavlinkComponent& _mavlink_component;

  bool send_message(mavlink_message_t& message) override {
    MavlinkMessage msg{message};
    messages.push_back(msg);
    return true;
  }
  [[nodiscard]] uint8_t get_own_system_id() const override {
    return _mavlink_component.m_sys_id;
  }
  [[nodiscard]] uint8_t get_own_component_id() const override {
    return _mavlink_component.m_comp_id;
  }
  [[nodiscard]] uint8_t get_system_id() const override {
    assert(true);
    return 0;
  }
  [[nodiscard]] Autopilot autopilot() const override {
    return Autopilot::Unknown;
  }

 public:
  std::vector<MavlinkMessage> messages;
};

}  // namespace mavsdk

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_
