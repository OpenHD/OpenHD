//
// Created by consti10 on 19.04.22.
//

#include "InternalTelemetry.h"
#include <iostream>
#include "OnboardComputerStatus.hpp"
#include "RebootUtil.hpp"
#include "WBStatisticsConverter.hpp"

InternalTelemetry::InternalTelemetry(bool runsOnAir) :RUNS_ON_AIR(runsOnAir),
                                                       MavlinkComponent(runsOnAir ? OHD_SYS_ID_AIR : OHD_SYS_ID_GROUND,MAV_COMP_ID_ONBOARD_COMPUTER) {
  wifibroadcastStatisticsUdpReceiver =
      std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
                                                  OHD_WIFIBROADCAST_STATISTICS_LOCAL_UDP_PORT,
                                                  [this](const uint8_t *payload,
                                                         const std::size_t payloadSize) {
                                                    processWifibroadcastStatisticsData(
                                                        payload,
                                                        payloadSize);
                                                  });
  wifibroadcastStatisticsUdpReceiver->runInBackground();
  logMessagesReceiver =
      std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
                                                  OHD_LOCAL_LOG_MESSAGES_UDP_PORT,
                                                  [this](const uint8_t *payload,
                                                         const std::size_t payloadSize) {
                                                    this->_status_text_accumulator.processLogMessageData(payload, payloadSize);
                                                  });
  logMessagesReceiver->runInBackground();
}

std::vector<MavlinkMessage> InternalTelemetry::generate_mavlink_messages() {
  std::cout<<"InternalTelemetry::generate_mavlink_messages()\n";
  std::vector<MavlinkMessage> ret;
  ret.push_back(OHDMessages::createHeartbeat(_sys_id,_comp_id));
  ret.push_back(OnboardComputerStatus::createOnboardComputerStatus(_sys_id,_comp_id));
  ret.push_back(generateWifibroadcastStatistics());
  ret.push_back(generateOpenHDVersion());
  // TODO remove for release
  //ret.push_back(MExampleMessage::position(mSysId,mCompId));
  // TODO remove for release
  //_status_text_accumulator.manually_add_message(RUNS_ON_AIR ? "HelloAir" : "HelloGround");
  const auto logs = generateLogMessages();
  MavlinkComponent::vec_append(ret,logs);
  //ret.insert(ret.end(), logs.begin(), logs.end());
  return ret;
}

std::vector<MavlinkMessage> InternalTelemetry::process_mavlink_message(const MavlinkMessage &msg) {
  // regarding reboot: https://mavlink.io/en/messages/common.html#MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN
  //if(msg.m.msgid==MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN){
  //}
  std::vector<MavlinkMessage> ret{};
  switch (msg.m.msgid) {
    case MAVLINK_MSG_ID_PING:{
      // We respond to ping messages
      auto response=handlePingMessage(msg);
      if(response.has_value()){
        ret.push_back(response.value());
      }
    }break;
    case MAVLINK_MSG_ID_COMMAND_LONG:{
      mavlink_command_long_t command;
      mavlink_msg_command_long_decode(&msg.m,&command);
      if(command.command==MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN){
        std::cout<<"Got shutdown command";
        RebootUtil::handlePowerCommand(false);
      }
      // TODO have an ack response.
    }break;
    default:
      break;
  }
  return ret;
}

void InternalTelemetry::processWifibroadcastStatisticsData(const uint8_t *payload, const std::size_t payloadSize) {
  //std::cout << "OHDTelemetryGenerator::processNewWifibroadcastStatisticsMessage: " << payloadSize << "\n";
  const auto msges=WBStatisticsConverter::parseRawDataSafe(payload,payloadSize);
  for(const auto msg:msges){
    lastWbStatisticsMessage[msg.radio_port] = msg;
  }
}

MavlinkMessage InternalTelemetry::generateWifibroadcastStatistics() const {
  OpenHDStatisticsWriter::Data data;
  // for now, write some crap
  data.radio_port = 0;
  data.count_p_all = 3;
  data.count_p_dec_err = 4;
  auto msg = WBStatisticsConverter::convertWbStatisticsToMavlink(data,_sys_id,_comp_id);
  return msg;
}

std::vector<MavlinkMessage> InternalTelemetry::generateLogMessages() {
  const auto messages=_status_text_accumulator.get_messages();
  std::vector<MavlinkMessage> ret;
  // limit to 5 to save bandwidth
  for(const auto& msg:messages){
    if (ret.size() < 5) {
      //std::cout<<"Msg:"<<msg.message<<"\n";
      MavlinkMessage mavMsg;
      StatusTextAccumulator::convert(mavMsg.m,msg,_sys_id,_comp_id);
      ret.push_back(mavMsg);
    } else {
      std::stringstream ss;
      ss << "Dropping log message " << msg.message << "\n";
      std::cout << ss.str();
    }
  }
  return ret;
}

MavlinkMessage InternalTelemetry::generateOpenHDVersion() const {
  MavlinkMessage msg;
  char bufferBigEnough[30]={};
  std::strncpy((char *)bufferBigEnough,"Dev-2.1",30);
  mavlink_msg_openhd_version_message_pack(_sys_id,_comp_id, &msg.m, bufferBigEnough);
  //mavlink_component_information_t x;
  return msg;
}


