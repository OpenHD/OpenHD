//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_

#include "MavlinkComponent.hpp"
#include <memory>
#include <vector>
#include <map>

/**
 * A system holds 0 or some components.
 */
class MavlinkSystem{
 public:
  MavlinkSystem(uint8_t sys_id):_sys_id(sys_id){

  }
  const uint8_t _sys_id;
  void add_component(std::shared_ptr<MavlinkComponent> component){
    if(components.find(component->_comp_id)!=components.end()){
      std::cerr<<"Error Component already exists";
      return;
    }
    components.insert_or_assign(component->_comp_id,component);
  }
  std::map<uint8_t,std::shared_ptr<MavlinkComponent>> components;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ROUTING_MAVLINKSYSTEM_H_
