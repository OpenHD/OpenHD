//
// Created by consti10 on 19.04.22.
//

#include "InternalTelemetry.h"
#include <iostream>
#include "SystemReadUtil.hpp"
#include "WBStatisticsConverter.hpp"

InternalTelemetry::InternalTelemetry(bool runsOnAir) : RUNS_ON_AIR(runsOnAir),
													   mSysId(runsOnAir ? OHD_SYS_ID_AIR : OHD_SYS_ID_GROUND) {
  wifibroadcastStatisticsUdpReceiver = std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
																				   OHD_WIFIBROADCAST_STATISTICS_LOCAL_UDP_PORT,
																				   [this](const uint8_t *payload,
																						  const std::size_t payloadSize) {
																					 processWifibroadcastStatisticsData(
																						 payload,
																						 payloadSize);
																				   });
  wifibroadcastStatisticsUdpReceiver->runInBackground();
  logMessagesReceiver = std::make_unique<SocketHelper::UDPReceiver>(SocketHelper::ADDRESS_LOCALHOST,
																	OHD_LOCAL_LOG_MESSAGES_UDP_PORT,
																	[this](const uint8_t *payload,
																		   const std::size_t payloadSize) {
																	  processLogMessageData(payload, payloadSize);
																	});
  logMessagesReceiver->runInBackground();
}

std::vector<MavlinkMessage> InternalTelemetry::generateUpdates() {
  std::vector<MavlinkMessage> ret;
  ret.push_back(OHDMessages::createHeartbeat(mSysId,mCompId));
  ret.push_back(SystemReadUtil::createOnboardComputerStatus(mSysId,mCompId));
  ret.push_back(generateWifibroadcastStatistics());
  ret.push_back(generateOpenHDVersion());
  // TODO remove for release
  ret.push_back(MExampleMessage::position(mSysId,mCompId));
  auto logs = generateLogMessages();
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
  auto msg = WBStatisticsConverter::convertWbStatisticsToMavlink(data, mSysId, mCompId);
  return msg;
}

std::vector<MavlinkMessage> InternalTelemetry::generateLogMessages() {
  std::vector<MavlinkMessage> ret;
  std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
  while (!bufferedLogMessages.empty()) {
	const auto msg = bufferedLogMessages.front();
	// for additional safety, we do not create more than 5 log messages per iteration, the rest is dropped
	// Otherwise we might run into bandwidth issues I suppose
	if (ret.size() < 5) {
	  MavlinkMessage mavMsg;
	  const uint64_t timestamp =
		  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	  mavlink_msg_openhd_log_message_pack(mSysId,
										  mCompId,
										  &mavMsg.m,
										  msg.level,
										  (const char *)&msg.message,
										  timestamp);
	  ret.push_back(mavMsg);
	} else {
	  std::stringstream ss;
	  ss << "Dropping log message " << msg.message << "\n";
	  std::cout << ss.str();
	}
	bufferedLogMessages.pop();
  }
  return ret;
}

MavlinkMessage InternalTelemetry::generateOpenHDVersion() const {
  MavlinkMessage msg;
  mavlink_msg_openhd_version_message_pack(mSysId, mCompId, &msg.m, "2.1");
  return msg;
}

void InternalTelemetry::processLogMessageData(const uint8_t *data, std::size_t dataLen) {
  //std::cout << "XX" << dataLen << "\n";
  //TODO fix safety
  if (dataLen == sizeof(OHDLocalLogMessage)) {
	OHDLocalLogMessage local_message{};
	memcpy((uint8_t *)&local_message, data, dataLen);
	const auto nullTerminatorFound = local_message.hasNullTerminator();
	if (!nullTerminatorFound) {
	  std::cerr << "Log message without null terminator\n";
	  return;
	}
	processLogMessage(local_message);
  } else {
	std::cerr << "Invalid size for local log message" << dataLen << " wanted:" << sizeof(OHDLocalLogMessage) << "\n";
  }
}

void InternalTelemetry::processLogMessage(OHDLocalLogMessage msg) {
  //std::cout<<"Log message:"<<msg.message<<"\n";
  std::lock_guard<std::mutex> guard(bufferedLogMessagesLock);
  bufferedLogMessages.push(msg);
}

std::optional<MavlinkMessage> InternalTelemetry::handlePingMessage(const MavlinkMessage &message) const {
  const auto msg=message.m;
  assert(msg.msgid==MAVLINK_MSG_ID_PING);
  mavlink_ping_t ping;
  mavlink_msg_ping_decode(&msg, &ping);
  // see https://mavlink.io/en/services/ping.html
  if(ping.target_system==0 && ping.target_component==0){
	std::cout<<"Got ping request\n";
	// Response to ping request.
	mavlink_message_t response_message;
	mavlink_msg_ping_pack(
		mSysId,
		mCompId,
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



