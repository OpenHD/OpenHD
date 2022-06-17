//
// Created by consti10 on 15.04.22.
//

#ifndef XMAVLINKSERVICE_MAV_HELPER_H
#define XMAVLINKSERVICE_MAV_HELPER_H

#include "mav_include.h"
#include <chrono>
#include <sstream>
#include <iostream>

namespace MExampleMessage {
// mostly from https://github.com/mavlink/mavlink/blob/master/examples/linux/mavlink_udp.c
static uint64_t microsSinceEpoch() {
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
static MavlinkMessage heartbeat(const int sys_id=255,const int comp_id=0) {
  MavlinkMessage msg{};
  mavlink_msg_heartbeat_pack(sys_id,
							 comp_id,
							 &msg.m,
							 MAV_TYPE_HELICOPTER,
							 MAV_AUTOPILOT_GENERIC,
							 MAV_MODE_GUIDED_ARMED,
							 0,
							 MAV_STATE_ACTIVE);
  return msg;
}
static MavlinkMessage position(const int sys_id=255,const int comp_id=0) {
  MavlinkMessage msg{};
  float position[6] = {};
  mavlink_msg_local_position_ned_pack(sys_id,comp_id, &msg.m, microsSinceEpoch(),
									  position[0], position[1], position[2],
									  position[3], position[4], position[5]);
  return msg;
}
static MavlinkMessage attitude(const int sys_id=255,const int comp_id=0) {
  MavlinkMessage msg{};
  mavlink_msg_attitude_pack(sys_id, comp_id, &msg.m, microsSinceEpoch(), 1.2, 1.7, 3.14, 0.01, 0.02, 0.03);
  return msg;
}
}

namespace OHDMessages {
/**
 * Both the air and ground service announce their existence with regular heartbeats
 * one can easily determine from which component the heartbeat comes by the corresponding sys id
 * TODO: should we also consider the component id ? Right now, we don't really seperate by components yet.
 * @param isAir if true, the sys_id (source) equals OHD air id, otherwise OHD ground id.
 */
static MavlinkMessage createHeartbeat(const int sys_id,const int comp_id) {
  MavlinkMessage heartbeat;
  mavlink_msg_heartbeat_pack(sys_id,
							 comp_id,
							 &heartbeat.m,
							 MAV_TYPE_GENERIC,
							 MAV_AUTOPILOT_GENERIC,
							 MAV_MODE_GUIDED_ARMED,
							 0,
							 MAV_STATE_ACTIVE);
  return heartbeat;
}

}

namespace MavlinkHelpers{

static std::string mavlink_status_to_string(const mavlink_status_t& mavlink_status){
  std::stringstream ss;
  ss<<"MavlinkStatus{";
  ss<<"msg_received:"<<(int)mavlink_status.msg_received<<",";
  ss<<"buffer_overrun:"<<(int)mavlink_status.buffer_overrun<<",";
  ss<<"parse_error:"<<(int)mavlink_status.parse_error<<",";
  ss<<"packet_idx:"<<(int)mavlink_status.packet_idx<<",";
  ss<<"current_rx_seq:"<<(int)mavlink_status.current_rx_seq<<",";
  ss<<"current_tx_seq:"<<(int)mavlink_status.current_tx_seq<<",";
  ss<<"packet_rx_success_count:"<<(int)mavlink_status.packet_rx_success_count<<",";
  ss<<" packet_rx_drop_count:"<<(int)mavlink_status. packet_rx_drop_count<<",";
  //ss<<":"<<(int)mavlink_status.<<",";
  return ss.str();
}

static std::string raw_content(const uint8_t* data,int data_len){
  std::stringstream ss;
  ss<<"[";
  for(int i=0;i<data_len;i++){
	ss<<(int)data[i]<<",";
  }
  ss<<"]";
  return ss.str();
}


static void parseAndLog(const uint8_t* data,int data_len,const uint8_t m_mavlink_channel,mavlink_status_t& receiveMavlinkStatus){
  int nMessages=0;
  mavlink_message_t msg;
  for (int i = 0; i < data_len; i++) {
	uint8_t res = mavlink_parse_char(m_mavlink_channel, (uint8_t)data[i], &msg, &receiveMavlinkStatus);
	if (res) {
	  MavlinkMessage message{msg};
	  //debugMavlinkMessage(message.m,"XYZ");
	  nMessages++;
	}
  }
  std::cout<<"parseAndLog:"<<" N messages:"<<nMessages<<"\n";
}

static void debugMavlinkPingMessage(const mavlink_message_t msg){
  mavlink_ping_t ping;
  mavlink_msg_ping_decode(&msg, &ping);
  std::stringstream ss;
  ss<<"Ping["<<"sys_id:"<<(int)msg.sysid<<" comp_id:"<<(int)msg.compid<<" seq:"<<ping.seq<<" target_system:"<<(int)ping.target_system<<" target_component:"<<(int)ping.target_component<<"]\n";
  std::cout<<ss.str();
}

}

static void debugMavlinkMessage(const mavlink_message_t &msg, const char *TAG) {
  printf("%s message with ID %d, sequence: %d from component %d of system %d\n",
		 TAG,
		 (int)msg.msgid,
		 msg.seq,
		 msg.compid,
		 msg.sysid);
}

static void lululululu(){
  //mavlink_msg_param_request_list_pack
  //mavlink_msg_param_ext_request_list_pack
}

#endif //XMAVLINKSERVICE_MAV_HELPER_H
