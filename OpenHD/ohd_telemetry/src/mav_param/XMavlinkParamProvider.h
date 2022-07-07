//
// Created by consti10 on 07.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_

#include "mavlink_settings/XSettingsComponent.h"
#include "routing/MavlinkComponent.hpp"

#include <memory>
#include <utility>

class XMavlinkParamProvider :public MavlinkComponent{
 public:
  explicit XMavlinkParamProvider(uint8_t sys_id,uint8_t comp_id,std::shared_ptr<openhd::XSettingsComponent> handler):
      MavlinkComponent(sys_id,comp_id),_handler(std::move(handler)){
  }

 private:
  std::shared_ptr<openhd::XSettingsComponent> _handler;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_XMAVLINKPARAMPROVIDER_H_
