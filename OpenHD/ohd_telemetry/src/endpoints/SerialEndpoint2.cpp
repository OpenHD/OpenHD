//
// Created by consti10 on 12.06.22.
//

#include "SerialEndpoint2.h"

SerialEndpoint2::SerialEndpoint2(std::string TAG, HWOptions options, bool enableDebug):
	MEndpoint(std::move(TAG)),
	m_options(std::move(options)),
	m_enable_debug(enableDebug){

  mavsdk=std::make_shared<mavsdk::Mavsdk>();
  mavsdk->add_serial_connection(m_options.linux_filename,m_options.baud_rate);
  mavsdk->subscribe_on_new_system([this]() {
	std::cout <<this->TAG<< " system found\n";
	auto system = this->mavsdk->systems().back();
	if (system->has_autopilot()) {
	  std::cout<<"Found autopilot\n";
	}
	if(systemFc!=nullptr){
	  std::cerr<<this->TAG<<" not expecting more than 1 system on UART\n";
	}else{
	  systemFc=system;
	  passtrougFC=std::make_shared<mavsdk::MavlinkPassthrough>(system);
	  passtrougFC->intercept_incoming_messages_async([this](mavlink_message_t& msg){
		//std::cout<<"Intercept:Got message:"<<msg.msgid<<"\n";
		this->parseNewDataEmulateForMavsdk(msg);
		return true;
	  });
	  /*passtrougFC->intercept_outgoing_messages_async([](mavlink_message_t& msg){
		std::cout<<"Intercept:send message:"<<msg.msgid<<"\n";
		return true;
	  });*/
	}
  });
  std::cout <<TAG<< " SerialEndpoint2 created " << m_options.linux_filename << ":"<<m_options.baud_rate<<"\n";
}

void SerialEndpoint2::sendMessageImpl(const MavlinkMessage &message) {
  // if we have discovered the component (got messages from it)
  if(passtrougFC!= nullptr){
	auto copy=message.m;
	passtrougFC->send_message(copy);
  }
}

