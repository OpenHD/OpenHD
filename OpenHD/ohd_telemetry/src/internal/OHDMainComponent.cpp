//
// Created by consti10 on 19.04.22.
//

#include "OHDMainComponent.h"

#include <iostream>
#include <openhd-global-constants.hpp>
#include <utility>

#include "OHDLinkStatisticsHelper.h"
#include "OnboardComputerStatus.hpp"
#include "RebootUtil.hpp"

OHDMainComponent::OHDMainComponent(
    OHDPlatform platform1,uint8_t parent_sys_id,
    bool runsOnAir,std::shared_ptr<openhd::ActionHandler> opt_action_handler) : m_platform(platform1),RUNS_ON_AIR(runsOnAir),
      m_opt_action_handler(std::move(opt_action_handler)),
	MavlinkComponent(parent_sys_id,MAV_COMP_ID_ONBOARD_COMPUTER) {
  m_console = openhd::log::create_or_get("t_main_c");
  assert(m_console);
  m_onboard_computer_status_provider=std::make_unique<OnboardComputerStatusProvider>(m_platform);
  m_log_messages_receiver =
      std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
                                                  openhd::LOCAL_LOG_MESSAGES_UDP_PORT,
                                                  [this](const uint8_t *payload,
                                                         const std::size_t payloadSize) {
                                                    this->m_status_text_accumulator.processLogMessageData(payload, payloadSize);
                                                  });
  m_log_messages_receiver->runInBackground();
  // suppress the warning until we get the first actually updated stats
  m_last_link_stats.is_air=RUNS_ON_AIR;
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_statistics_register([this](openhd::link_statistics::StatsAirGround stats_air_ground){
      this->set_link_statistics(stats_air_ground);
    });
  }
}

OHDMainComponent::~OHDMainComponent() {
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_statistics_register(nullptr);
  }
}

std::vector<MavlinkMessage> OHDMainComponent::generate_mavlink_messages() {
  //m_console->debug("InternalTelemetry::generate_mavlink_messages()");
  std::vector<MavlinkMessage> ret;
  ret.push_back(MavlinkComponent::create_heartbeat());
  OHDUtil::vec_append(ret,m_onboard_computer_status_provider->get_current_status_as_mavlink_message(
          m_sys_id, m_comp_id));
  OHDUtil::vec_append(ret, generate_mav_wb_stats());
  //ret.push_back(generateOpenHDVersion());
  //ret.push_back(MExampleMessage::position(mSysId,mCompId));
  //_status_text_accumulator.manually_add_message(RUNS_ON_AIR ? "HelloAir" : "HelloGround");
  const auto logs = generateLogMessages();
  OHDUtil::vec_append(ret,logs);
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::process_mavlink_messages(std::vector<MavlinkMessage> messages) {
  std::vector<MavlinkMessage> ret{};
  for(const auto& msg:messages){
    switch (msg.m.msgid) { // NOLINT(cppcoreguidelines-narrowing-conversions)
      case MAVLINK_MSG_ID_TIMESYNC:{
        // makes ping obsolete
        auto response= handle_timesync_message(msg);
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
          if(command.target_system== m_sys_id){
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
              m_console->debug("Unimplemented");
            }
          }
        }else if(command.command==MAV_CMD_REQUEST_MESSAGE){
          const auto requested_message_id=static_cast<uint32_t>(command.param1);
          m_console->debug("Someone requested a specific message: {}",requested_message_id);
          if(requested_message_id==MAVLINK_MSG_ID_OPENHD_VERSION_MESSAGE){
            m_console->info("Sent OpenHD version");
            ret.push_back(generate_ohd_version());
          }
        }else if(command.command==OPENHD_CMD_INITIATE_CHANNEL_SEARCH){
          if(RUNS_ON_AIR){
            m_console->debug("Scan channels is only a feature for ground unit");
            break;
          }else{
            const auto freq_bands=static_cast<uint32_t>(command.param1);
            const auto channel_widths=static_cast<uint32_t>(command.param2);
            m_console->debug("OPENHD_CMD_INITIATE_CHANNEL_SEARCH {} {}",freq_bands,channel_widths);
            if((freq_bands==0 || freq_bands==1 || freq_bands==2) &&
                (channel_widths==0 || channel_widths==1 || channel_widths==2)){
              const bool scan_2g=freq_bands==0 || freq_bands==1;
              const bool scan_5g=freq_bands==0 || freq_bands==2;
              const bool scan_20Mhz=channel_widths==0 || channel_widths==1;
              const bool scan_40Mhz=channel_widths==0 || channel_widths==2;
              if(m_opt_action_handler){
                m_opt_action_handler->action_wb_link_scan_channels_handle({scan_2g,scan_5g,scan_20Mhz,scan_40Mhz});
                ret.push_back(ack_command(msg.m.sysid,msg.m.compid,command.command));
              }
            }
          }
        }
        // TODO have an ack response.
      }break;
      default:
        break;
    }
  }
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generate_mav_wb_stats(){
  //m_console->debug("OHDMainComponent::generate_mav_wb_stats");
  std::vector<MavlinkMessage> ret;
  const auto latest_stats=get_latest_link_statistics();
  if(RUNS_ON_AIR!=latest_stats.is_air){
    m_console->warn("Mismatch air/ground");
    return ret;
  }
  // stats for all the wifi card(s)
  int card_index=0;
  for(const auto& card_stats : latest_stats.cards){
    if(!card_stats.exists_in_openhd){
      // skip non active cards
      continue;
    }
    MavlinkMessage msg= openhd::LinkStatisticsHelper::pack_card(
        m_sys_id, m_comp_id, card_index, card_stats);
    card_index++;
    ret.push_back(msg);
  }
  ret.push_back(openhd::LinkStatisticsHelper::pack_link_general(
      m_sys_id, m_comp_id,latest_stats.monitor_mode_link));
  ret.push_back(openhd::LinkStatisticsHelper::pack_tele(
      m_sys_id, m_comp_id, latest_stats.telemetry));
  if(RUNS_ON_AIR){
    for(const auto& stats : latest_stats.stats_wb_video_air){
      ret.push_back(openhd::LinkStatisticsHelper::pack_vid_air(
          m_sys_id, m_comp_id, stats));
    }
  }else{
    for(const auto& ground_video: latest_stats.stats_wb_video_ground){
      ret.push_back(openhd::LinkStatisticsHelper::pack_vid_gnd(
          m_sys_id, m_comp_id, ground_video));
    }
  }
  return ret;
}

std::vector<MavlinkMessage> OHDMainComponent::generateLogMessages() {
  const auto messages= m_status_text_accumulator.get_messages();
  std::vector<MavlinkMessage> ret;
  // limit to 5 to save bandwidth
  for(const auto& msg:messages){
    if (ret.size() < 5) {
      MavlinkMessage mavMsg;
      StatusTextAccumulator::convert(mavMsg.m,msg, m_sys_id, m_comp_id);
      ret.push_back(mavMsg);
    } else {
      m_console->debug("Dropping log message {}",msg.msg_as_string());
    }
  }
  return ret;
}

MavlinkMessage OHDMainComponent::generate_ohd_version(const std::string& commit_hash) const {
  MavlinkMessage msg;
  char bufferBigEnough[30]={};
  std::strncpy((char *)bufferBigEnough,openhd::VERSION_NUMBER_STRING,30);
  char bufferBigEnough2[30]={};
  std::strncpy((char *)bufferBigEnough2,commit_hash.c_str(),30);
  mavlink_msg_openhd_version_message_pack(m_sys_id, m_comp_id, &msg.m, bufferBigEnough,bufferBigEnough2);
  //mavlink_component_information_t x;
  return msg;
}

void OHDMainComponent::set_link_statistics(openhd::link_statistics::StatsAirGround stats){
  std::lock_guard<std::mutex> guard(m_last_link_stats_mutex);
  m_last_link_stats=stats;
}

openhd::link_statistics::StatsAirGround OHDMainComponent::get_latest_link_statistics() {
  std::lock_guard<std::mutex> guard(m_last_link_stats_mutex);
  return m_last_link_stats;
}

MavlinkMessage OHDMainComponent::ack_command(const uint8_t source_sys_id,const uint8_t source_comp_id,uint16_t command_id) {
  MavlinkMessage ret{};
  mavlink_msg_command_ack_pack(m_sys_id, m_comp_id,&ret.m,command_id,MAV_RESULT_ACCEPTED,255,0,source_sys_id,source_comp_id);
  return ret;
}

std::optional<MavlinkMessage> OHDMainComponent::handle_timesync_message(const MavlinkMessage &message) {
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
    mavlink_msg_timesync_encode(m_sys_id, m_comp_id,&response_message,&rsync);
    return MavlinkMessage{response_message};
  }
  return std::nullopt;
}
