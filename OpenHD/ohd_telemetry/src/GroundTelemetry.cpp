//
// Created by consti10 on 13.04.22.
//

#include "GroundTelemetry.h"
#include <iostream>
#include "mav_helper.h"

GroundTelemetry::GroundTelemetry() {
  /*tcpGroundCLient=std::make_unique<TCPEndpoint>(OHD_GROUND_CLIENT_TCP_PORT);
  tcpGroundCLient->registerCallback([this](MavlinkMessage& msg){
	  onMessageGroundStationClients(msg);
  });*/
  udpGroundClient =
	  std::make_unique<UDPEndpoint>("GroundStationUDP", OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN);
  udpGroundClient->registerCallback([this](MavlinkMessage &msg) {
	onMessageGroundStationClients(msg);
  });
  // hacky, start breoadcasting the existence of the OHD ground station
  // udpGroundClient->startHeartBeat(OHD_SYS_ID_GROUND,0);
  // any message coming in via wifibroadcast is a message from the air pi
  udpWifibroadcastEndpoint = UDPEndpoint::createEndpointForOHDWifibroadcast(false);
  udpWifibroadcastEndpoint->registerCallback([this](MavlinkMessage &msg) {
	onMessageAirPi(msg);
  });
  std::cout << "Created GroundTelemetry\n";
}

void GroundTelemetry::onMessageAirPi(MavlinkMessage &message) {
  debugMavlinkMessage(message.m,"GroundTelemetry::onMessageAirPi");
  const mavlink_message_t &m = message.m;
  // we do not need to forward heartbeat messages coming from the air telemetry service
  if (m.msgid == MAVLINK_MSG_ID_HEARTBEAT && m.sysid == OHD_SYS_ID_AIR) {
	// heartbeat coming from the air service
	return;
  }
  // for now, forward everything
  sendMessageGroundStationClients(message);
}

void GroundTelemetry::onMessageGroundStationClients(MavlinkMessage &message) {
  debugMavlinkMessage(message.m, "GroundTelemetry::onMessageGroundStationClients");
  const auto &msg = message.m;
  // for now, forward everything
  sendMessageAirPi(message);
}

void GroundTelemetry::sendMessageGroundStationClients(MavlinkMessage &message) {
  debugMavlinkMessage(message.m, "GroundTelemetry::sendMessageGroundStationClients");
  // forward via TCP or UDP
  if (tcpGroundCLient) {
	tcpGroundCLient->sendMessage(message);
  }
  if (udpGroundClient) {
	udpGroundClient->sendMessage(message);
  }
}

void GroundTelemetry::sendMessageAirPi(MavlinkMessage &message) {
  // transmit via wifibroadcast
  if (udpWifibroadcastEndpoint) {
	udpWifibroadcastEndpoint->sendMessage(message);
  }
}

void GroundTelemetry::loopInfinite(const bool enableExtendedLogging) {
  while (true) {
	//std::cout << "GroundTelemetry::loopInfinite()\n";
	// for debugging, check if any of the endpoints is not alive
	if (enableExtendedLogging && udpWifibroadcastEndpoint) {
	  udpWifibroadcastEndpoint->debugIfAlive();
	}
	if (enableExtendedLogging && udpGroundClient) {
	  udpGroundClient->debugIfAlive();
	}
	// send messages to the ground station in regular intervals, includes heartbeat.
	// everything else is handled by the callbacks and their threads
	auto ohdTelemetryMessages = ohdTelemetryGenerator.generateUpdates();
	for (auto &msg: ohdTelemetryMessages) {
	  sendMessageGroundStationClients(msg);
	}
	std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

