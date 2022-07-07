//
// Created by consti10 on 07.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_

#include "mavlink_settings/XSettingsComponent.h"
#include "routing/MavlinkComponent.hpp"

#include <memory>
#include <utility>

class MavlinkParameterReceiver;

class XMavlinkParamProvider :public MavlinkComponent{
 public:
  explicit XMavlinkParamProvider(MavlinkSystem& parent,uint8_t comp_id,std::shared_ptr<openhd::XSettingsComponent> handler);
  // override from component
  std::vector<MavlinkMessage> process_mavlink_message(const MavlinkMessage &msg)override;
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;
 private:
  std::shared_ptr<openhd::XSettingsComponent> _handler;
  std::shared_ptr<MavlinkParameterReceiver> _mavlink_parameter_receiver;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
