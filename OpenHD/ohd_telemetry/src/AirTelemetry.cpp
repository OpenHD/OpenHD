//
// Created by consti10 on 13.04.22.
//

#include "AirTelemetry.h"
#include "mav_helper.h"


AirTelemetry::AirTelemetry(std::string fcSerialPort) {
  serialEndpoint = std::make_unique<SerialEndpoint>("FCSerial",SerialEndpoint::HWOptions{fcSerialPort, 115200});
  serialEndpoint->registerCallback([this](MavlinkMessage &msg) {
	this->onMessageFC(msg);
  });
  // any message coming in via wifibroadcast is a message from the ground pi
  wifibroadcastEndpoint = UDPEndpoint::createEndpointForOHDWifibroadcast(true);
  wifibroadcastEndpoint->registerCallback([this](MavlinkMessage &msg) {
	onMessageGroundPi(msg);
  });
  std::cout << "Created AirTelemetry\n";
}

void AirTelemetry::sendMessageFC(MavlinkMessage &message) {
  serialEndpoint->sendMessage(message);
  if(message.m.msgid==MAVLINK_MSG_ID_PING){
	std::cout<<"Sent ping to FC\n";
	MavlinkHelpers::debugMavlinkPingMessage(message.m);
  }
}

void AirTelemetry::sendMessageGroundPi(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"AirTelemetry::sendMessageGroundPi");
  // broadcast the mavlink message via wifibroadcast
  wifibroadcastEndpoint->sendMessage(message);
}

void AirTelemetry::onMessageFC(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"AirTelemetry::onMessageFC");
  sendMessageGroundPi(message);
  // handling a message from the FC is really easy - we just forward it to the ground pi.
  if(message.m.msgid==MAVLINK_MSG_ID_PING){
	std::cout<<"Got ping from FC\n";
  }
}

void AirTelemetry::onMessageGroundPi(MavlinkMessage &message) {
  const mavlink_message_t &m = message.m;
  // we do not need to forward heartbeat messages coming from the ground telemetry service,
  // They solely have a debugging purpose such that one knows the other service is alive.
  if (m.msgid == MAVLINK_MSG_ID_HEARTBEAT && m.sysid == OHD_SYS_ID_GROUND) {
	// heartbeat coming from the ground service
	return;
  }
  // for now, do it as simple as possible
  sendMessageFC(message);
  // temporarily, handle ping messages
  if(m.msgid==MAVLINK_MSG_ID_PING){
	auto response=ohdTelemetryGenerator.handlePingMessage(message);
	if(response.has_value()){
	  sendMessageGroundPi(response.value());
	}
  }
}

void AirTelemetry::loopInfinite(const bool enableExtendedLogging) {
  while (true) {
	//std::cout << "AirTelemetry::loopInfinite()\n";
	// for debugging, check if any of the endpoints is not alive
	if (enableExtendedLogging && wifibroadcastEndpoint) {
	  std::cout<<wifibroadcastEndpoint->createInfo();
	}
	if (enableExtendedLogging && serialEndpoint) {
	  std::cout<<serialEndpoint->createInfo();
	}
	// send messages to the ground pi in regular intervals, includes heartbeat.
	// everything else is handled by the callbacks and their threads
	auto ohdTelemetryMessages = ohdTelemetryGenerator.generateUpdates();
	for (auto &msg: ohdTelemetryMessages) {
	  sendMessageGroundPi(msg);
	}
	// send out in X second intervals
	std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

std::string AirTelemetry::createDebug() const {
  std::stringstream ss;
  //ss<<"AT:\n";
  if ( wifibroadcastEndpoint) {
	ss<<wifibroadcastEndpoint->createInfo();
  }
  if (serialEndpoint) {
	ss<<serialEndpoint->createInfo();
  }
  return ss.str();
}
