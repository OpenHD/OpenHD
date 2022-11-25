//
// Created by consti10 on 16.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_

#include "../mav_include.h"
#include "openhd-link-statistics.hpp"

namespace openhd::LinkStatisticsHelper{

static MavlinkMessage
pack0(const uint8_t system_id,const uint8_t component_id,int card_index,
                             const openhd::link_statistics::StatsPerCard& card_stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_monitor_mode_wifi_card_t tmp;
  tmp.card_index=card_index;
  tmp.rx_rssi=card_stats.rx_rssi;
  tmp.count_p_received=card_stats.count_p_received;
  tmp.count_p_injected=card_stats.count_p_injected;
  tmp.dummy0=0;
  tmp.dummy1=0;
  mavlink_msg_openhd_stats_monitor_mode_wifi_card_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage
pack1(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsTelemetry& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_telemetry_t tmp;
  mavlink_msg_openhd_stats_telemetry_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage
pack2(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoAir& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_air_t tmp;
  mavlink_msg_openhd_stats_wb_video_air_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}

static MavlinkMessage
pack3(const uint8_t system_id,const uint8_t component_id,const openhd::link_statistics::StatsWBVideoGround& stats){
  MavlinkMessage msg;
  mavlink_openhd_stats_wb_video_ground_t tmp;
  mavlink_msg_openhd_stats_wb_video_ground_encode(system_id,component_id,&msg.m,&tmp);
  return msg;
}




}
#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_OHDLINKSTATISTICSHELPER_H_
