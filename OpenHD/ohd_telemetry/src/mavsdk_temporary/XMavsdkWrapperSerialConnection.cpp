//
// Created by consti10 on 09.07.22.
//

#include "XMavsdkWrapperSerialConnection.h"

#include <utility>
#include "openhd-util-filesystem.hpp"

namespace mavsdk {

XMavsdkWrapperSerialConnection::XMavsdkWrapperSerialConnection(
    std::optional<std::string> path, int baudrate, bool flow_control)
    : MEndpoint("SerialFC"),_path(std::move(path)),_baudrate(baudrate),_flow_control(flow_control) {
  _for_mavsdk_receiver_callback=[this](mavlink_message_t& message, Connection* connection){
    MEndpoint::parseNewDataEmulateForMavsdk(message);
  };
  establish_connection_thread = std::make_unique<std::thread>(&XMavsdkWrapperSerialConnection::constantConnect, this);
}

bool XMavsdkWrapperSerialConnection::sendMessageImpl(
    const MavlinkMessage& message) {
  std::lock_guard<std::mutex> lock(_mutex);
  if(_serial_connection){
	const auto result=_serial_connection->send_message(message.m);
	if(!result){
	  std::cout<<"XMavsdkWrapperSerialConnection:: serial most likely disconnected\n";
	  // cannot send - probably disconnected
	  _serial_connection->stop();
	  _serial_connection= nullptr;
	}
  }
  return true;
}

std::optional<std::string> XMavsdkWrapperSerialConnection::check_serial_file_handles() {
  if(_path.has_value()){
	if(!OHDFilesystemUtil::exists(_path.value().c_str())){
	  std::cout<<"UART file does not exist\n";
	  return std::nullopt;
	}
	return _path.value();
  }else{
	// most common paths
	const std::string path1="/dev/ttyACM0";
	const std::string path2="/dev/ttyACM1";
	if(OHDFilesystemUtil::exists(path1.c_str())){
	  return path1;
	}
	if(OHDFilesystemUtil::exists(path2.c_str())){
	  return path2;
	}
	return std::nullopt;
  }
}

[[noreturn]] void XMavsdkWrapperSerialConnection::constantConnect() {
  while (true){
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard<std::mutex> lock(_mutex);
	if(_serial_connection){
	  continue;
	}
	const auto path_opt=check_serial_file_handles();
	if(path_opt.has_value()){
	  const auto& path=path_opt.value();
	  std::cout<<"XMavsdkWrapperSerialConnection::start "<<path;
	  _serial_connection=std::make_unique<SerialConnection>(_for_mavsdk_receiver_callback,path,_baudrate,_flow_control);
	  const auto result=_serial_connection->start();
	  std::cout<<"XMavsdkWrapperSerialConnection:"<<result<<"\n";
	  if(result!=mavsdk::ConnectionResult::Success){
		std::cerr<<"Cannot create serial connection on "<<path;
		_serial_connection= nullptr;
		continue;
	  }
	}else{
	  //std::cout<<"Cannot find valid UART file handle\n";
	}
  }
}


}