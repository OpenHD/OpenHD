//
// Created by consti10 on 19.04.22.
//

#include <iostream>

#include "OHDMainComponent.h"
#include "OnboardComputerStatus.hpp"
#include "RebootUtil.hpp"
#include "OHDLinkStatisticsHelper.h"

OHDMainComponent::OHDMainComponent(
    OHDPlatform platform1,uint8_t parent_sys_id,
    bool runsOnAir,std::shared_ptr<openhd::ActionHandler> opt_action_handler) :
	platform(platform1),RUNS_ON_AIR(runsOnAir),_opt_action_handler(opt_action_handler),
	MavlinkComponent(parent_sys_id,MAV_COMP_ID_ONBOARD_COMPUTER) {
  m_console = openhd::loggers::create_or_get("tele_main_comp");
  assert(m_console);
  m_console->set_level(spd::level::debug);
  m_onboard_computer_status_provider=std::make_unique<OnboardComputerStatusProvider>(platform);
  logMessagesReceiver =
      std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
                                                  OHD_LOCAL_LOG_MESSAGES_UDP_PORT,
                                                  [this](const uint8_t *payload,
                                                         const std::size_t payloadSize) {
                                                    this->_status_text_accumulator.processLogMessageData(payload, payloadSize);
                                                  });
  logMessagesReceiver->runInBackground();
}

std::vector<MavlinkMessage> OHDMainComponent::generate_mavlink_messages() {
  //m_console->debug("InternalTelemetry::generate_mavlink_messages()\n";
  std::vector<MavlinkMessage> ret;
  ret.push_back(MavlinkComponent::create_heartbeat());
  MavlinkComponent::vec_append(ret,m_onboard_computer_status_provider->get_current_status_as_mavlink_message(_sys_id,_comp_id));
  MavlinkComponent::vec_append(ret,generateWifibroadcastStatistics());
  //ret.push_back(generateOpenHDVersion());
  // TODO remove for release
  //ret.push_back(MExampleMessage::position(mSysId,mCompId));
  // TODO remove for release
  //_status_text_accumulator.manually_add_message(RUNS_ON_AIR ? "HelloAir" : "HelloGround");
  const auto logs = generateLogMessages();
  MavlinkComponent::vec_append(ret,logs);
  //ret.insert(ret.end(), logs.begin(), logs.end());
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::process_mavlink_message(const MavlinkMessage &msg) {
  std::vector<MavlinkMessage> ret{};
  switch (msg.m.msgid) { // NOLINT(cppcoreguidelines-narrowing-conversions)
	// Obsolete
    /*case MAVLINK_MSG_ID_PING:{
      // We respond to ping messages
      auto response=handlePingMessage(msg);
      if(response.has_value()){
        ret.push_back(response.value());
      }
    }break;*/
	case MAVLINK_MSG_ID_TIMESYNC:{
	  // makes ping obsolete
	  auto response=handleTimeSyncMessage(msg);
	  if(response.has_value()){
		ret.push_back(response.value());
	  }
	}break;

    case MAVLINK_MSG_ID_COMMAND_LONG:{
      mavlink_command_long_t command;
      mavlink_msg_command_long_decode(&msg.m,&command);
      m_console->debug("Got MAVLINK_MSG_ID_COMMAND_LONG: {} {}",command.command,static_cast<uint32_t>(command.param1));
      if(command.command==MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN){
        //https://mavlink.io/en/messages/common.html#MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN
        m_console->debug("Got shutdown command");
		if(command.target_system==_sys_id){
		  // we are a companion computer, so we use param2 to get the actual action
		  const auto action_for_companion=command.param2;
		  if(action_for_companion>0){
			ret.push_back(ack_command(msg.m.sysid,msg.m.compid,command.command));
			const bool shutdownOnly=action_for_companion==2;
			RebootUtil::handle_power_command_async(std::chrono::seconds(1),shutdownOnly);
		  }
		  // dirty, we don't have a custom message for that yet
		  if(command.param3==1){
			ret.push_back(ack_command(msg.m.sysid,msg.m.compid,command.command));
			if(_opt_action_handler){
			  _opt_action_handler->action_restart_wb_streams_handle();
			}
		  }
		}
      }else if(command.command==MAV_CMD_REQUEST_MESSAGE){
        const auto requested_message_id=static_cast<uint32_t>(command.param1);
        m_console->debug("Someone requested a specific message: {}",requested_message_id);
        if(requested_message_id==MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE){
          m_console->info("Sent OpenHD version");
          ret.push_back(generateOpenHDVersion());
        }
      }
      // TODO have an ack response.
    }break;
    default:
      break;
  }
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generateWifibroadcastStatistics(){
  std::lock_guard<std::mutex> guard(_last_link_stats_mutex);
  std::vector<MavlinkMessage> ret;
  // stats for all the wifi card(s)
  for(int i=0;i<_last_link_stats.stats_all_cards.size();i++){
	const auto card_stats=_last_link_stats.stats_all_cards.at(i);
	if(!card_stats.exists_in_openhd){
	  // skip non active cards
	  continue;
	}
	MavlinkMessage msg=openhd::LinkStatisticsHelper::wifibroadcast_wifi_card_pack(_sys_id,_comp_id,0,card_stats);
	ret.push_back(msg);
  }
  {
	const auto& all_stats=_last_link_stats.stats_total_all_streams;
	MavlinkMessage msg=openhd::LinkStatisticsHelper::stats_total_all_wifibroadcast_streams_pack(_sys_id,_comp_id,all_stats);
	ret.push_back(msg);
  }
  {
	if(!RUNS_ON_AIR){
	  // Video fex rx stats only on ground
	  if(_last_link_stats.stats_video_stream0_rx.has_value()){
		const auto& stats_video_stream_rx=_last_link_stats.stats_video_stream0_rx.value();
		MavlinkMessage msg=openhd::LinkStatisticsHelper::fec_link_rx_statistics_pack(_sys_id,_comp_id,0,stats_video_stream_rx);
		ret.push_back(msg);
	  }
	}
  }
  // stats per ling
  //mavlink_msg_openhd_fec_link_rx_statistics_pack()
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generateLogMessages() {
  const auto messages=_status_text_accumulator.get_messages();
  std::vector<MavlinkMessage> ret;
  // limit to 5 to save bandwidth
  for(const auto& msg:messages){
    if (ret.size() < 5) {
      MavlinkMessage mavMsg;
      StatusTextAccumulator::convert(mavMsg.m,msg,_sys_id,_comp_id);
      ret.push_back(mavMsg);
    } else {
      std::stringstream ss;
      ss << "Dropping log message " << msg.message;
      m_console->debug(ss.str());
    }
  }
  return ret;
}

MavlinkMessage OHDMainComponent::generateOpenHDVersion() const {
  MavlinkMessage msg;
  char bufferBigEnough[30]={};
  std::strncpy((char *)bufferBigEnough,OHD_VERSION_NUMBER_STRING,30);
  mavlink_msg_openhd_version_message_pack(_sys_id,_comp_id, &msg.m, bufferBigEnough);
  //mavlink_component_information_t x;
  return msg;
}

void OHDMainComponent::set_link_statistics(openhd::link_statistics::AllStats stats){
  //m_console->debug("OHDMainComponent::set_link_statistics");
  std::lock_guard<std::mutex> guard(_last_link_stats_mutex);
  _last_link_stats=stats;
}

MavlinkMessage OHDMainComponent::ack_command(const uint8_t source_sys_id,const uint8_t source_comp_id,uint16_t command_id) {
  MavlinkMessage ret{};
  mavlink_msg_command_ack_pack(_sys_id,_comp_id,&ret.m,command_id,MAV_RESULT_ACCEPTED,255,0,source_sys_id,source_comp_id);
  return ret;
}

std::optional<MavlinkMessage> OHDMainComponent::handleTimeSyncMessage(const MavlinkMessage &message) {
  const auto msg=message.m;
  assert(msg.msgid==MAVLINK_MSG_ID_TIMESYNC);
  mavlink_timesync_t tsync;
  mavlink_msg_timesync_decode(&msg, &tsync);
  if(tsync.tc1==0){
	// request, pack response
	mavlink_timesync_t rsync;
	rsync.tc1 = get_time_microseconds() * 1000;
	rsync.ts1 = tsync.ts1;
	mavlink_message_t response_message;
	mavlink_msg_timesync_encode(_sys_id,_comp_id,&response_message,&rsync);
	return MavlinkMessage{response_message};
  }
  return std::nullopt;
}

MavlinkMessage OHDMainComponent::generateRcControlMessage() const {
  MavlinkMessage ret{};
  mavlink_rc_channels_override_t mavlink_rc_channels_override{};
  mavlink_rc_channels_override.chan1_raw=0;
  mavlink_msg_rc_channels_override_encode(_sys_id,_comp_id,&ret.m,&mavlink_rc_channels_override);
  return ret;
}
