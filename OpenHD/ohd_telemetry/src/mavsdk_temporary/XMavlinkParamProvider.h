//
// Created by consti10 on 07.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_

#include "mavlink_settings/ISettingsComponent.h"
#include "routing/MavlinkComponent.hpp"
#include <memory>
#include <utility>

// mavsdk
#include "mavlink_parameter_receiver.h"
#include "mavlink_message_handler.h"
#include "sender_wrapper.h"

class XMavlinkParamProvider :public MavlinkComponent{
 public:
  // Note : only with manually_call_set_ready==true you can create the param provider,
  // then add settings on the fly and finally - when all settings are added - call "ready for c0mmunication".
  explicit XMavlinkParamProvider(uint8_t sys_id,uint8_t comp_id,const std::vector<openhd::Setting>& settings,
								 bool manually_call_set_ready=false);
  void add_param(const openhd::Setting& setting);
  // only usable when manually_set_ready is true
  void add_params(const std::vector<openhd::Setting>& settings);
  void set_ready();
  // override from component
  std::vector<MavlinkMessage> process_mavlink_message(const MavlinkMessage &msg)override;
  // override from component
  std::vector<MavlinkMessage> generate_mavlink_messages() override;
 private:
  // mavsdk
  std::shared_ptr<mavsdk::SenderWrapper> _sender;
  std::shared_ptr<mavsdk::MavlinkMessageHandler> _mavlink_message_handler;
  std::shared_ptr<mavsdk::MavlinkParameterReceiver> _mavlink_parameter_receiver;
 private:
  std::mutex _mutex{};
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
