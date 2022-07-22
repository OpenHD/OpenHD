//
// Created by consti10 on 09.07.22.
//

#include "XMavsdkWrapperSerialConnection.h"
#include "openhd-util-filesystem.hpp"

namespace mavsdk {

XMavsdkWrapperSerialConnection::XMavsdkWrapperSerialConnection(
    const std::string& path, int baudrate, bool flow_control)
    : MEndpoint("SerialFC"),_path() {
  _for_mavsdk_receiver_callback=[this](mavlink_message_t& message, Connection* connection){
    MEndpoint::parseNewDataEmulateForMavsdk(message);
  };
  _serial_connection=std::make_unique<SerialConnection>(_for_mavsdk_receiver_callback,path,baudrate,flow_control);
  establish_connection_thread = std::make_unique<std::thread>(&XMavsdkWrapperSerialConnection::constantConnect, this);
}

void XMavsdkWrapperSerialConnection::sendMessageImpl(
    const MavlinkMessage& message) {
  std::lock_guard<std::mutex> lock(_mutex);
  if(!_started){
	return;
  }
  const auto result=_serial_connection->send_message(message.m);
  if(!result){
    // cannot send - probably disconnected
  }
}
void XMavsdkWrapperSerialConnection::constantConnect() {
  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if(!OHDFilesystemUtil::exists(_path.c_str())){
	  std::cout<<"UART file does not exist\n";
	  continue;
	}
    const auto result=_serial_connection->start();
	std::cout<<"XMavsdkWrapperSerialConnection:"<<result<<"\n";
    if(result!=mavsdk::ConnectionResult::Success){
	  continue;
    }
	_started=true;
	return;
  }
}

}