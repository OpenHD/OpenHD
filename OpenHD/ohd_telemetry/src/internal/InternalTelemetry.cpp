//
// Created by consti10 on 19.04.22.
//

#include "InternalTelemetry.h"
#include <iostream>
#include "OnboardComputerStatus.hpp"
#include "WBStatisticsConverter.hpp"

InternalTelemetry::InternalTelemetry(bool runsOnAir) :RUNS_ON_AIR(runsOnAir),
                                                       MavlinkComponent(runsOnAir ? OHD_SYS_ID_AIR : OHD_SYS_ID_GROUND,0) {
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

std::vector<MavlinkMessage> InternalTelemetry::generateUpdates() {
  std::cout<<"InternalTelemetry::generateUpdates()\n";
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
  ret.insert(ret.end(), logs.begin(), logs.end());
  return ret;
}

bool InternalTelemetry::handleMavlinkCommandIfPossible(const MavlinkMessage &msg) {
  // regarding reboot: https://mavlink.io/en/messages/common.html#MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN
  //if(msg.m.msgid==MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN){
  //}
  return false;
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
  mavlink_msg_openhd_version_message_pack(_sys_id,_comp_id, &msg.m, "2.1");
  return msg;
}

std::optional<MavlinkMessage> InternalTelemetry::handlePingMessage(const MavlinkMessage &message) const {
  const auto msg=message.m;
  assert(msg.msgid==MAVLINK_MSG_ID_PING);
  mavlink_ping_t ping;
  mavlink_msg_ping_decode(&msg, &ping);
  // see https://mavlink.io/en/services/ping.html
  if(ping.target_system==0 && ping.target_component==0){
    //std::cout<<"Got ping request\n";
    // Response to ping request.
    mavlink_message_t response_message;
    mavlink_msg_ping_pack(
        _sys_id,_comp_id,
        &response_message,
        ping.time_usec,
        ping.seq,
        msg.sysid,
        msg.compid);
    return MavlinkMessage{response_message};
  }else{
    // answer from ping request
    return std::nullopt;
  }
}

