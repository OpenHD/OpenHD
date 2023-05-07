//
// Created by consti10 on 06.05.23.
//

#ifndef OPENHD_CHANNELMAPPINGUTIL_H
#define OPENHD_CHANNELMAPPINGUTIL_H

#include <array>
#include <string>
#include "openhd_spdlog.h"

// Util methods for simple channel mapping, where each channel's input can be defined by the user.
namespace openhd{

static constexpr auto N_MAPPED_CHANNELS=8;

// Channel mapping: just look at the default to understand ;)
using CHAN_MAP=std::array<int,N_MAPPED_CHANNELS>;

static bool validate_channel_mapping(const CHAN_MAP& chan_map) {
  for(const auto& el:chan_map){ // NOLINT(readability-use-anyofallof)
    if(el<0 || el>=N_MAPPED_CHANNELS){
      openhd::log::get_default()->warn("Channel mapping not a valid value{}",el);
      return false;
    }
  }
  return true;
}

static std::optional<CHAN_MAP>
convert_string_to_channel_mapping(const std::string& input) {
  auto split_into_substrings=OHDUtil::split_into_substrings(input,',');
  if(split_into_substrings.size()!=N_MAPPED_CHANNELS){
    openhd::log::get_default()->warn("Channel mapping wrong n channels:{}",split_into_substrings.size());
    return std::nullopt;
  }
  CHAN_MAP parsed_as_int{};
  for(int i=0;i<N_MAPPED_CHANNELS;i++){
    const auto as_int=OHDUtil::string_to_int(split_into_substrings[i]);
    if(!as_int.has_value())return std::nullopt;
    parsed_as_int[i]=as_int.value();
  }
  if(!validate_channel_mapping(parsed_as_int))return std::nullopt;
  return parsed_as_int;
}

static CHAN_MAP get_default_channel_mapping() {
  CHAN_MAP ret{};
  for(int i=0;i<N_MAPPED_CHANNELS;i++){
    ret[i]=i;
  }
  return ret;
}

static std::array<uint16_t,18> remap_channels(const std::array<uint16_t,18>& channels,const CHAN_MAP& chan_map){
  std::array<uint16_t,18> ret{};
  for(int i=0;i<18;i++){
    // Better be safe than sorry regarding bounds checking (even though we shouldn't ever be out of bounds)
    if(i<chan_map.size()){
      const auto channel_to_use=chan_map[i];
      if(channel_to_use>=0 && channel_to_use < channels.size()){
        ret[i]=channels[channel_to_use];
      }
    }else{
      ret[i]=channels[i];
    }
  }
  return ret;
}


}
#endif  // OPENHD_CHANNELMAPPINGUTIL_H
