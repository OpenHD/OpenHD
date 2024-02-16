//
// Created by consti10 on 08.07.22.
//

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
