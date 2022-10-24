//
// Created by consti10 on 13.04.22.
//

#include "AirTelemetry.h"
#include "mav_helper.h"
#include "mavsdk_temporary/XMavlinkParamProvider.h"
#include <chrono>
#include "openhd-util-filesystem.hpp"
#include "openhd-rpi-os-configure-vendor-cam.hpp"

AirTelemetry::AirTelemetry(OHDPlatform platform,std::shared_ptr<openhd::ActionHandler> opt_action_handler): _platform(platform),MavlinkSystem(OHD_SYS_ID_AIR) {
  _airTelemetrySettings=std::make_unique<openhd::AirTelemetrySettingsHolder>();
  setup_uart();
  // any message coming in via wifibroadcast is a message from the ground pi
  wifibroadcastEndpoint = UDPEndpoint::createEndpointForOHDWifibroadcast(true);
  wifibroadcastEndpoint->registerCallback([this](MavlinkMessage &msg) {
	onMessageGroundPi(msg);
  });
  _ohd_main_component=std::make_shared<OHDMainComponent>(_platform,_sys_id,true,opt_action_handler);
  components.push_back(_ohd_main_component);
  //
  generic_mavlink_param_provider=std::make_shared<XMavlinkParamProvider>(_sys_id,MAV_COMP_ID_ONBOARD_COMPUTER);
  // NOTE: We don't call set ready yet, since we have to wait until other modules have provided
  // all their paramters.
  generic_mavlink_param_provider->add_params(get_all_settings());
  components.push_back(generic_mavlink_param_provider);
  std::cout << "Created AirTelemetry\n";
}

void AirTelemetry::sendMessageFC(const MavlinkMessage &message) {
  std::lock_guard<std::mutex> guard(_serialEndpointMutex);
  if(serialEndpoint){
    serialEndpoint->sendMessage(message);
  }else{
    //std::cout<<"Cannot send message to FC\n";
  }
}

void AirTelemetry::sendMessageGroundPi(const MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"AirTelemetry::sendMessageGroundPi");
  // broadcast the mavlink message via wifibroadcast
  wifibroadcastEndpoint->sendMessage(message);
}

void AirTelemetry::onMessageFC(MavlinkMessage &message) {
  //debugMavlinkMessage(message.m,"AirTelemetry::onMessageFC");
  // Note: No OpenHD component ever talks to the FC, FC is completely passed through
  sendMessageGroundPi(message);
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
  // any data created by an OpenHD component on the air pi only needs to be sent to the ground pi, the FC cannot do anything with it anyways.
  std::lock_guard<std::mutex> guard(components_lock);
  for(auto& component: components){
	const auto responses=component->process_mavlink_message(message);
	for(const auto& response:responses){
	  sendMessageGroundPi(response);
	}
  }
}

[[noreturn]] void AirTelemetry::loopInfinite(const bool enableExtendedLogging) {
  const auto log_intervall=std::chrono::seconds(5);
  const auto loop_intervall=std::chrono::milliseconds(500);
  auto last_log=std::chrono::steady_clock::now();
  while (true) {
	const auto loopBegin=std::chrono::steady_clock::now();
	if(std::chrono::steady_clock::now()-last_log>=log_intervall){
	  // State debug logging
	  last_log=std::chrono::steady_clock::now();
	  //std::cout << "AirTelemetry::loopInfinite()\n";
	  // for debugging, check if any of the endpoints is not alive
	  if (enableExtendedLogging && wifibroadcastEndpoint) {
		std::cout<<wifibroadcastEndpoint->createInfo();
	  }
	  std::lock_guard<std::mutex> guard(_serialEndpointMutex);
	  if (enableExtendedLogging && serialEndpoint) {
		std::cout<<serialEndpoint->createInfo();
	  }
	}
	// send messages to the ground pi in regular intervals, includes heartbeat.
	// everything else is handled by the callbacks and their threads
	{
	  std::lock_guard<std::mutex> guard(components_lock);
	  for(auto& component:components){
		const auto messages=component->generate_mavlink_messages();
		for(const auto& msg:messages){
		  sendMessageGroundPi(msg);
		}
	  }
	}
	const auto loopDelta=std::chrono::steady_clock::now()-loopBegin;
	if(loopDelta>loop_intervall){
	  // We can't keep up with the wanted loop interval
	  std::stringstream ss;
	  ss<<"Warning AirTelemetry cannot keep up with the wanted loop interval. Took:"
	  <<std::chrono::duration_cast<std::chrono::milliseconds>(loopDelta).count()<<"ms\n";
	  std::cout<<ss.str();
	}else{
	  const auto sleepTime=loop_intervall-loopDelta;
	  // send out in X second intervals
	  std::this_thread::sleep_for(loop_intervall);
	}
  }
}

std::string AirTelemetry::createDebug(){
  std::stringstream ss;
  //ss<<"AT:\n";
  if ( wifibroadcastEndpoint) {
	ss<<wifibroadcastEndpoint->createInfo();
  }
  std::lock_guard<std::mutex> guard(_serialEndpointMutex);
  if (serialEndpoint) {
	ss<<serialEndpoint->createInfo();
  }
  return ss.str();
}

void AirTelemetry::add_settings_generic(const std::vector<openhd::Setting>& settings) {
  std::lock_guard<std::mutex> guard(components_lock);
  generic_mavlink_param_provider->add_params(settings);
  std::cout<<"Added parameter component\n";
}

void AirTelemetry::set_link_statistics(openhd::link_statistics::AllStats stats) {
  if(_ohd_main_component){
	_ohd_main_component->set_link_statistics(stats);
  }
}

void AirTelemetry::settings_generic_ready() {
  generic_mavlink_param_provider->set_ready();
}

void AirTelemetry::add_camera_component(const int camera_index, const std::vector<openhd::Setting> &settings) {
  assert(camera_index>=0 && camera_index<2);
  const auto cam_comp_id=MAV_COMP_ID_CAMERA+camera_index;
  auto param_server=std::make_shared<XMavlinkParamProvider>(_sys_id,cam_comp_id,true);
  param_server->add_params(settings);
  param_server->set_ready();
  std::lock_guard<std::mutex> guard(components_lock);
  components.push_back(param_server);
  std::cout<<"Added camera component\n";
}

std::vector<openhd::Setting> AirTelemetry::get_all_settings() {
  std::vector<openhd::Setting> ret{};
  auto c_fc_uart_connection_type=[this](std::string,int value) {
	if(!openhd::validate_uart_connection_type(value)){
	  return false;
	}
	_airTelemetrySettings->unsafe_get_settings().fc_uart_connection_type=value;
	_airTelemetrySettings->persist();
	setup_uart();
	return true;
  };
  auto c_fc_uart_baudrate=[this](std::string,int value) {
	if(!openhd::validate_uart_baudrate(value)){
	  return false;
	}
	_airTelemetrySettings->unsafe_get_settings().fc_uart_baudrate=value;
	_airTelemetrySettings->persist();
	setup_uart();
	return true;
  };
  auto c_rpi_os_camera_configuration=[this](std::string,std::string value){
    if(!openhd::rpi::os::validate_cam_config_settings_string(value)){
      // reject, not a valid value
      return false;
    }
    const auto curr_value=openhd::rpi::os::get_current_cam_config();
    const auto new_value=openhd::rpi::os::cam_config_from_string(value);
    if(curr_value!=new_value){
      // TODO apply
      std::cerr<<"TODO change cam config from "<<openhd::rpi::os::cam_config_to_string(curr_value)
                <<" to "<<openhd::rpi::os::cam_config_to_string(new_value)<<"\n";
      openhd::rpi::os::update_current_cam_config(new_value);
    }
    return true;
  };
  ret.push_back(openhd::Setting{openhd::FC_UART_CONNECTION_TYPE,openhd::IntSetting{static_cast<int>(_airTelemetrySettings->get_settings().fc_uart_connection_type),
																		   c_fc_uart_connection_type}});
  ret.push_back(openhd::Setting{openhd::FC_UART_BAUD_RATE,openhd::IntSetting{static_cast<int>(_airTelemetrySettings->get_settings().fc_uart_baudrate),
																	 c_fc_uart_baudrate}});
  // This way one can switch between different OS configuration(s) that then provide access to different
  // vendor-specific camera(s) - hacky/dirty I know ;/
  if(_platform.platform_type==PlatformType::RaspberryPi){
    ret.push_back(openhd::Setting{"V_OS_CAM_CONFIG",openhd::StringSetting {openhd::rpi::os::cam_config_to_string(openhd::rpi::os::get_current_cam_config()),
                                                                        c_rpi_os_camera_configuration}});
  }
  return ret;
}

// Every time the UART configuration changes, we just re-start the UART (if it was already started)
// This properly handles all the cases, e.g cleaning up an existing uart connection if set.
void AirTelemetry::setup_uart() {
  assert(_airTelemetrySettings);
  const auto fc_uart_connection_type=_airTelemetrySettings->get_settings().fc_uart_connection_type;
  const auto fc_uart_baudrate=_airTelemetrySettings->get_settings().fc_uart_baudrate;
  assert(openhd::validate_uart_connection_type(fc_uart_connection_type));
  // Disable the currently running uart configuration, if there is any
  std::lock_guard<std::mutex> guard(_serialEndpointMutex);
  if(serialEndpoint!=nullptr) {
	std::cout<<"Stopping already existing FC UART\n";
	serialEndpoint->stop();
	serialEndpoint.reset();
	serialEndpoint=nullptr;
  }
  if(fc_uart_connection_type==openhd::UART_CONNECTION_TYPE_DISABLE){
	// No uart enabled, we've already cleaned it up though
	std::cout<<"FC UART disabled\n";
	return;
  }else{
	std::cout<<"FC UART enable - begin\n";
	SerialEndpoint::HWOptions options{};
	options.linux_filename=openhd::uart_fd_from_connection_type(fc_uart_connection_type).value();
	options.baud_rate=fc_uart_baudrate;
	options.flow_control= false;
	serialEndpoint=std::make_unique<SerialEndpoint>("SerialEndpointUARTFC",options);
	serialEndpoint->registerCallback([this](MavlinkMessage &msg) {
	  this->onMessageFC(msg);
	});
	std::cout<<"FC UART enable - end\n";
  }
}
