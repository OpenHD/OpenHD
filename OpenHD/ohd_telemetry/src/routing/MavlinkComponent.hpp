//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_

class MavlinkComponent{
 public:
  MavlinkComponent(uint8_t sys_id,uint8_t comp_id):_sys_id(sys_id),_comp_id(comp_id){}
  const uint8_t _sys_id;
  const uint8_t _comp_id;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKCOMPONENT_H_
