//
// Created by consti10 on 08.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_

#include "sender.h"

namespace mavsdk{

class SenderWrapper:public Sender{
  bool send_message(mavlink_message_t& message){
    MavlinkMessage msg{message};
    messages.push_back(msg);
    return true;
  }
  [[nodiscard]] uint8_t get_own_system_id() const{
    return 0;
  }
  [[nodiscard]] uint8_t get_own_component_id() const {
    return 0;
  }
  [[nodiscard]] uint8_t get_system_id() const {
    assert(true);
    return 0;
  }
  [[nodiscard]] Autopilot autopilot() const {
    return Autopilot::Unknown;
  }

 public:
  std::vector<MavlinkMessage> messages;
};

}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_SENDERWRAPPER_H_
