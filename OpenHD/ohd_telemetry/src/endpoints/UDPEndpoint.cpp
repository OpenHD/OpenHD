//
// Created by consti10 on 14.04.22.
//

#include "UDPEndpoint.h"
#include "openhd-global-constants.hpp"
#include <utility>

UDPEndpoint::UDPEndpoint(const std::string& TAG, const int senderPort, const int receiverPort,std::string senderIp,std::string receiverIp) :
	MEndpoint(TAG),
	SEND_PORT(senderPort), RECV_PORT(receiverPort) {
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  /*if (senderPort == receiverPort) {
	throw std::invalid_argument("UDPEndpoint - cannot send and receive on same UDP port\n");
  }*/
  if (SEND_PORT >= 0) {
	transmitter = std::make_unique<SocketHelper::UDPForwarder>(senderIp, SEND_PORT);
  }
  if (RECV_PORT >= 0) {
	const auto cb = [this](const uint8_t *payload, const std::size_t payloadSize)mutable {
	  this->parseNewData(payload, (int)payloadSize);
	};
	receiver = std::make_unique<SocketHelper::UDPReceiver>(receiverIp, RECV_PORT, cb);
	receiver->runInBackground();
  }
  std::stringstream ss;
  ss<<" UDPEndpoint created send: "<<senderIp<<":" << senderPort << " recv: "<<receiverIp<<":" << receiverPort;
  m_console->info(ss.str());
}

UDPEndpoint::~UDPEndpoint() {
  if(receiver){
    receiver->stopBackground();
  }
}

bool UDPEndpoint::sendMessageImpl(const MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"UDPEndpoint::sendMessage");
  if (transmitter != nullptr) {
	const auto data = message.pack();
	transmitter->forwardPacketViaUDP(data.data(), data.size());
	return true;
  } else {
	m_console->debug("UDPEndpoint::sendMessage with no transmitter");
  }
  return false;
}

std::unique_ptr<UDPEndpoint> UDPEndpoint::createEndpointForOHDWifibroadcast(const bool isAir) {
  const std::string tag = std::string("udp_endp").append(isAir ? "A" : "G");
  int txp;
  int rxp;
  // tx and rx is swapped on air and ground to debug this module locally.
  if (isAir) {
	txp = OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
	rxp = OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
  } else {
	txp = OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
	rxp = OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
  }
  return std::make_unique<UDPEndpoint>(tag, txp, rxp);
}
