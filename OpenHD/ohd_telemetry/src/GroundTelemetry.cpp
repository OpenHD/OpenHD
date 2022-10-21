//
// Created by consti10 on 13.04.22.
//

#include "GroundTelemetry.h"

#include "mavsdk_temporary//XMavlinkParamProvider.h"

#include <iostream>
#include <chrono>

#include "mav_helper.h"

GroundTelemetry::GroundTelemetry(OHDPlatform platform,std::shared_ptr<openhd::ActionHandler> opt_action_handler): _platform(platform),MavlinkSystem(OHD_SYS_ID_GROUND) {
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
  _ohd_main_component=std::make_shared<OHDMainComponent>(_platform,_sys_id,false,opt_action_handler);
  components.push_back(_ohd_main_component);
  //
  // NOTE: We don't call set ready yet, since we have to wait until other modules have provided
  // all their parameters.
  generic_mavlink_param_provider=std::make_shared<XMavlinkParamProvider>(_sys_id,MAV_COMP_ID_ONBOARD_COMPUTER);
  components.push_back(generic_mavlink_param_provider);
  // temporary
  m_joystick_reader=std::make_unique<JoystickReader>();
  std::cout << "Created GroundTelemetry\n";
}

void GroundTelemetry::onMessageAirPi(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"GroundTelemetry::onMessageAirPi");
  const mavlink_message_t &m = message.m;
  // All messages we get from the Air pi (they might come from the AirPi itself or the FC connected to the air pi)
  // get forwarded straight to all the client(s) connected to the ground station.
  sendMessageGroundStationClients(message);
  // Note: No OpenHD component ever talks to another OpenHD component or the FC, so we do not
  // need to do anything else here.
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
  //if (tcpGroundCLient) {
  //	tcpGroundCLient->sendMessage(message);
  //}
  if (udpGroundClient) {
	udpGroundClient->sendMessage(message);
  }
  std::lock_guard<std::mutex> guard(other_udp_ground_stations_lock);
  for (auto const& [key, val] : _other_udp_ground_stations){
	val->sendMessage(message);
  }
}

void GroundTelemetry::sendMessageAirPi(const MavlinkMessage &message) {
  //debugMavlinkMessage(message.m, "GroundTelemetry::sendMessageAirPi");
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
	const auto loopBegin=std::chrono::steady_clock::now();
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
	const auto loopDelta=std::chrono::steady_clock::now()-loopBegin;
	if(loopDelta>loop_intervall){
	  // We can't keep up with the wanted loop interval
	  std::stringstream ss;
	  ss<<"Warning GroundTelemetry cannot keep up with the wanted loop interval. Took:"
		<<std::chrono::duration_cast<std::chrono::milliseconds>(loopDelta).count()<<"ms\n";
	  std::cout<<ss.str();
	}else{
	  const auto sleepTime=loop_intervall-loopDelta;
	  // send out in X second intervals
	  std::this_thread::sleep_for(loop_intervall);
	}
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

void GroundTelemetry::add_settings_generic(const std::vector<openhd::Setting>& settings) {
  std::lock_guard<std::mutex> guard(components_lock);
  generic_mavlink_param_provider->add_params(settings);
  std::cout<<"Added parameter component\n";
}

void GroundTelemetry::set_link_statistics(openhd::link_statistics::AllStats stats) {
  if(_ohd_main_component){
	_ohd_main_component->set_link_statistics(stats);
  }
}

void GroundTelemetry::settings_generic_ready() {
  generic_mavlink_param_provider->set_ready();
}

void GroundTelemetry::add_external_ground_station_ip(const std::string& ip_openhd,const std::string& ip_dest_device) {
  std::stringstream ss;
  ss<<"GroundTelemetry::add_external_ground_station_ip:ip_openhd:["<<ip_openhd<<",ip_dest_device:"<<ip_dest_device<<"]\n";
  std::cout<<ss.str();
  if(!OHDUtil::is_valid_ip(ip_openhd) || !OHDUtil::is_valid_ip(ip_dest_device)){
	std::cerr<<"These are no valid IPs, skipping\n";
	return;
  }
  std::lock_guard<std::mutex> guard(other_udp_ground_stations_lock);
  assert(OHDUtil::is_valid_ip(ip_openhd));
  assert(OHDUtil::is_valid_ip(ip_dest_device));
  const std::string identifier=ip_openhd+"_"+ip_dest_device;
  const auto port_offset=_other_udp_ground_stations.size()+1;
  auto tmp=std::make_shared<UDPEndpoint2>("GroundStationUDPX",OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN+port_offset,
										  ip_dest_device,ip_openhd);
  tmp->registerCallback([this](MavlinkMessage &mavlinkMessage){
	// Now this is weird, but somehow we get a lot of junk from QGroundControll on android ??!!
	// QGroundControll defaults to 255
	// QOpenHD defaults to 225;
	const bool is_from_ground_controll=mavlinkMessage.m.sysid==255 || mavlinkMessage.m.sysid==225;
	if(!is_from_ground_controll){
	  // This can't really be a message from a ground controll application
	  //std::cout<<"Dropping message\n";
	  return;
	}
	//debugMavlinkMessage(mavlinkMessage.m, "GroundTelemetry::external GCS message");
	onMessageGroundStationClients(mavlinkMessage);
  });
  _other_udp_ground_stations[identifier]=tmp;
}

void GroundTelemetry::remove_external_ground_station_ip(const std::string &ip_openhd,const std::string& ip_dest_device) {
  std::stringstream ss;
  ss<<"GroundTelemetry::remove_external_ground_station_ip:ip_openhd:["<<ip_openhd<<",ip_dest_device:"<<ip_dest_device<<"]\n";
  std::cout<<ss.str();
  if(!OHDUtil::is_valid_ip(ip_openhd) || !OHDUtil::is_valid_ip(ip_dest_device)){
	std::cerr<<"These are no valid IPs, skipping\n";
	return;
  }
  std::lock_guard<std::mutex> guard(other_udp_ground_stations_lock);
  assert(OHDUtil::is_valid_ip(ip_openhd));
  assert(OHDUtil::is_valid_ip(ip_dest_device));
  const std::string identifier=ip_openhd+"_"+ip_dest_device;
  // shared pointer will clean up for us
  _other_udp_ground_stations.erase(identifier);
}

