//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_

#include <map>
#include <memory>
#include <vector>

/**
 * A system has a sys id and allows adding components.
 */
class MavlinkSystem {
 public:
  explicit MavlinkSystem(uint8_t sys_id) : _sys_id(sys_id) {}
  const uint8_t _sys_id;
  /*void add_component(std::shared_ptr<MavlinkComponent> component){
    if(components.find(component->_comp_id)!=components.end()){
      std::cerr<<"Error Component already exists";
      return;
    }
    components.insert_or_assign(component->_comp_id,component);
  }
  std::map<uint8_t,std::shared_ptr<MavlinkComponent>> components;*/
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_
