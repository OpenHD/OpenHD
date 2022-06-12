//
// Created by consti10 on 14.04.22.
//

#include "UDPEndpoint.h"
#include "openhd-global-constants.h"
#include <utility>

UDPEndpoint::UDPEndpoint(const std::string& TAG, const int senderPort, const int receiverPort,std::string senderIp,std::string receiverIp
	,bool extra) :
	MEndpoint(TAG),
	SEND_PORT(senderPort), RECV_PORT(receiverPort) {
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
  std::cout <<TAG<< " UDPEndpoint created send: "<<senderIp<<":" << senderPort << " recv: "<<receiverIp<<":" << receiverPort << "\n";
  if(extra){
	transmitter->lula("127.0.0.1",14551);
  }
}

void UDPEndpoint::sendMessage(const MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"UDPEndpoint::sendMessage");
  if (transmitter != nullptr) {
	const auto data = message.pack();
	//std::cout<<"XSend:"<<data.size()<<" "<<MavlinkHelpers::raw_content(data.data(),data.size())<<"\n";
	transmitter->forwardPacketViaUDP(data.data(), data.size());
	//std::cout<<"AAA\n";
	//parseNewData(data.data(),data.size());
  } else {
	std::cerr << "UDPEndpoint::sendMessage with no transmitter\n";
  }
}

std::unique_ptr<UDPEndpoint> UDPEndpoint::createEndpointForOHDWifibroadcast(const bool isAir) {
  const std::string tag = std::string("WBUDPEndpoint").append(isAir ? "A" : "G");
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






