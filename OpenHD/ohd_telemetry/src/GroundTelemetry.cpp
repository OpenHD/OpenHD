//
// Created by consti10 on 13.04.22.
//

#include "GroundTelemetry.h"

#include "mavsdk_temporary//XMavlinkParamProvider.h"

#include <iostream>
#include <chrono>

#include "mav_helper.h"

GroundTelemetry::GroundTelemetry(OHDPlatform platform): _platform(platform),MavlinkSystem(OHD_SYS_ID_GROUND) {
  /*tcpGroundCLient=std::make_unique<TCPEndpoint>(OHD_GROUND_CLIENT_TCP_PORT);
  tcpGroundCLient->registerCallback([this](MavlinkMessage& msg){
          onMessageGroundStationClients(msg);
  });*/
  /*udpGroundClient =std::make_unique<UDPEndpoint>("GroundStationUDP",
                                                                                                 OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN,
                                                                                                 "127.0.0.1","127.0.0.1",true);//127.0.0.1
  udpGroundClient->registerCallback([this](MavlinkMessage &msg) {
        onMessageGroundStationClients(msg);
  });*/
  udpGroundClient =
      std::make_unique<UDPEndpoint2>("GroundStationUDP",OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN,
                                     "127.0.0.1","127.0.0.1");
  udpGroundClient->registerCallback([this](MavlinkMessage &msg) {
    onMessageGroundStationClients(msg);
  });
  // any message coming in via wifibroadcast is a message from the air pi
  udpWifibroadcastEndpoint = UDPEndpoint::createEndpointForOHDWifibroadcast(false);
  udpWifibroadcastEndpoint->registerCallback([this](MavlinkMessage &msg) {
    onMessageAirPi(msg);
  });
  _ohd_main_component=std::make_shared<OHDMainComponent>(_platform,_sys_id,false);
  components.push_back(_ohd_main_component);
  std::cout << "Created GroundTelemetry\n";
}

void GroundTelemetry::onMessageAirPi(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"GroundTelemetry::onMessageAirPi");
  const mavlink_message_t &m = message.m;
  // All messages we get from the Air pi (they might come from the AirPi itself or the FC connected to the air pi)
  // get forwarded straight to all the client(s) connected to the ground station.
  sendMessageGroundStationClients(message);
}

void GroundTelemetry::onMessageGroundStationClients(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m, "GroundTelemetry::onMessageGroundStationClients");
  const auto &msg = message.m;
  // All messages from the ground station(s) are forwarded to the air unit.
  sendMessageAirPi(message);
  // OpenHD components running on the ground station don't need to talk to the air unit.
  // This is not exactly following the mavlink routing standard, but saves a lot of bandwidth.
  std::lock_guard<std::mutex> guard(components_lock);
  for(auto& component:components){
    const auto responses=component->process_mavlink_message(message);
    for(const auto& response:responses){
      // for now, send to the ground station clients only
      sendMessageGroundStationClients(response);
    }
  }
}

void GroundTelemetry::sendMessageGroundStationClients(const MavlinkMessage &message) {
  //debugMavlinkMessage(message.m, "GroundTelemetry::sendMessageGroundStationClients");
  // forward via TCP or UDP
  if (tcpGroundCLient) {
	tcpGroundCLient->sendMessage(message);
  }
  if (udpGroundClient) {
	udpGroundClient->sendMessage(message);
  }
  std::lock_guard<std::mutex> guard(other_udp_ground_stations_lock);
  for(const auto& other_client:other_udp_ground_stations){
	other_client->sendMessage(message);
  }
}

void GroundTelemetry::sendMessageAirPi(const MavlinkMessage &message) {
  // transmit via wifibroadcast
  if (udpWifibroadcastEndpoint) {
	udpWifibroadcastEndpoint->sendMessage(message);
  }
}

[[noreturn]] void GroundTelemetry::loopInfinite(const bool enableExtendedLogging) {
  const auto log_intervall=std::chrono::seconds(5);
  const auto loop_intervall=std::chrono::milliseconds(500);
  auto last_log=std::chrono::steady_clock::now();
  while (true) {
    if(std::chrono::steady_clock::now()-last_log>=log_intervall) {
      last_log = std::chrono::steady_clock::now();
      std::cout << "GroundTelemetry::loopInfinite()\n";
      // for debugging, check if any of the endpoints is not alive
      if (enableExtendedLogging && udpWifibroadcastEndpoint) {
        std::cout<<udpWifibroadcastEndpoint->createInfo();
      }
      if (enableExtendedLogging && udpGroundClient) {
        std::cout<<udpGroundClient->createInfo();
      }
    }
    // send messages to the ground station in regular intervals, includes heartbeat.
    // everything else is handled by the callbacks and their threads
	{
	  std::lock_guard<std::mutex> guard(components_lock);
	  for(auto& component:components){
		assert(component);
		const auto messages=component->generate_mavlink_messages();
		for(const auto& msg:messages){
		  // r.n no ground unit component needs to talk to the air unit directly.
		  sendMessageGroundStationClients(msg);
		  if(msg.m.msgid==MAVLINK_MSG_ID_HEARTBEAT && msg.m.compid==MAV_COMP_ID_ONBOARD_COMPUTER){
			// but we send heartbeats to the air pi anyways, just to keep the link active.
			//std::cout<<"Heartbeat sent to air unit\n";
			sendMessageAirPi(msg);
		  }
		}
	  }
	}
    std::this_thread::sleep_for(loop_intervall);
  }
}

std::string GroundTelemetry::createDebug() const {
  std::stringstream ss;
  //ss<<"GT:\n";
  if (udpWifibroadcastEndpoint) {
	std::cout<<udpWifibroadcastEndpoint->createInfo();
  }
  if (udpGroundClient) {
	std::cout<<udpGroundClient->createInfo();
  }
  return ss.str();
}

void GroundTelemetry::add_settings_component(
    int comp_id, std::shared_ptr<openhd::XSettingsComponent> glue) {
  auto param_server=std::make_shared<XMavlinkParamProvider>(_sys_id,comp_id,std::move(glue));
  std::lock_guard<std::mutex> guard(components_lock);
  components.push_back(param_server);
  std::cout<<"Added parameter component\n";
}

void GroundTelemetry::set_link_statistics(openhd::link_statistics::AllStats stats) {
  if(_ohd_main_component){
	_ohd_main_component->set_link_statistics(stats);
  }
}

void GroundTelemetry::add_external_ground_station_ip(std::string ip_openhd,std::string ip_dest_device) {
  std::stringstream ss;
  ss<<"GroundTelemetry::add_external_ground_station_ip:ip_openhd:["<<ip_openhd<<",ip_dest_device:"<<ip_dest_device<<"]\n";
  std::cout<<ss.str();
  std::lock_guard<std::mutex> guard(other_udp_ground_stations_lock);
  /*auto tmp=std::make_shared<UDPEndpoint2>("GroundStationUDPX",OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN,
										  ip_openhd,ip_dest_device);
  other_udp_ground_stations.emplace_back(tmp);*/
  auto tmp=std::make_shared<UDPEndpoint>("XUDPE",14550,-1,ip_dest_device,"");
  other_udp_ground_stations.emplace_back(tmp);
}
