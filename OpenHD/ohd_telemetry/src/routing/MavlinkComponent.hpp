//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_

#include <cassert>
#include <optional>
#include <utility>

#include "MavlinkSystem.hpp"
#include "mav_include.h"

// A component has a (parent) sys id and its own component id (unique per
// system). It processes and/or creates mavlink messages.
class MavlinkComponent {
 public:
  MavlinkComponent(uint8_t sys_id, uint8_t comp_id,
                   std::optional<MAV_TYPE> mav_type = std::nullopt)
      : m_sys_id(sys_id), m_comp_id(comp_id), _mav_type(mav_type) {}
  // MavlinkComponent(MavlinkSystem& parent,uint8_t comp_id):
  //      _parent(parent),_sys_id(_parent._sys_id),_comp_id(comp_id){}
  // MavlinkSystem& _parent;
  const uint8_t m_sys_id;
  const uint8_t m_comp_id;
  const std::optional<MAV_TYPE> _mav_type = std::nullopt;
  /**
   * Call this with any mavlink message received, the component can then decide
   * if he can use this message or not.
   * @return a list of mavlink messages that were created as a response.Empty
   * unless the given message needs a response.
   */
  virtual std::vector<MavlinkMessage> process_mavlink_messages(
      std::vector<MavlinkMessage> messages) = 0;
  /**
   * The parent should call this method in regular intervals and send out the
   * generated mavlink messages. This is for fire and forget messages. For
   * example, a component might return the heartbeat(s) here.
   */
  virtual std::vector<MavlinkMessage> generate_mavlink_messages() = 0;

 protected:
  // These are protected, and MUST be called in the implementation(s) process
  // message method if the component supports them. For example, one might not
  // want a component to respond to any ping messages.
  /**
   * Handle a ping message. Ping is a bit complicated in mavlink, since sys and
   * comp id of 0 have special meaning. This implementation is similar to the
   * one from mavsdk.
   * @param message the ping message to handle
   * @return a ping response message if needed, no message otherwise. (not every
   * ping needs a response) NOTE: Replaced by timesync, see discussion here:
   * https://github.com/ArduPilot/ardupilot/issues/21311 Timesync can do ping +
   * even more (if wanted)
   */
  /*[[nodiscard]] std::optional<MavlinkMessage> handlePingMessage(const
  MavlinkMessage &message) const { const auto msg=message.m;
    assert(msg.msgid==MAVLINK_MSG_ID_PING);
    mavlink_ping_t ping;
    mavlink_msg_ping_decode(&msg, &ping);
    // see https://mavlink.io/en/services/ping.html
    if(ping.target_system==0 && ping.target_component==0){
      //std::cout<<"Got ping request\n";
      // Response to ping request.
      mavlink_message_t response_message;
      mavlink_msg_ping_pack(
          _sys_id,_comp_id,
          &response_message,
          ping.time_usec,
          ping.seq,
          msg.sysid,
          msg.compid);
      return MavlinkMessage{response_message};
    }else{
      // answer from ping request
      return std::nullopt;
    }
  }*/
  // Convenient method to create a heartbeat, for use in the implementation
  [[nodiscard]] MavlinkMessage create_heartbeat() const {
    MavlinkMessage heartbeat;
    const MAV_TYPE mav_type =
        _mav_type.has_value() ? _mav_type.value() : MAV_TYPE_GENERIC;
    mavlink_msg_heartbeat_pack(m_sys_id, m_comp_id, &heartbeat.m, mav_type,
                               MAV_AUTOPILOT_INVALID, 0, 0, MAV_STATE_ACTIVE);
    return heartbeat;
  }
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_
