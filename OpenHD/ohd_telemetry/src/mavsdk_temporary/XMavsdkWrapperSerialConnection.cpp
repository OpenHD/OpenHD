//
// Created by consti10 on 09.07.22.
//

#include "XMavsdkWrapperSerialConnection.h"

namespace mavsdk {

XMavsdkWrapperSerialConnection::XMavsdkWrapperSerialConnection(
    const std::string& path, int baudrate, bool flow_control)
    : MEndpoint("SerialFC") {
  _for_mavsdk_receiver_callback=[this](mavlink_message_t& message, Connection* connection){
    MEndpoint::parseNewDataEmulateForMavsdk(message);
  };
  _serial_connection=std::make_unique<SerialConnection>(_for_mavsdk_receiver_callback,path,baudrate,flow_control);
  establish_connection_thread = std::make_unique<std::thread>(&XMavsdkWrapperSerialConnection::constantConnect, this);
}

void XMavsdkWrapperSerialConnection::sendMessageImpl(
    const MavlinkMessage& message) {
  std::lock_guard<std::mutex> lock(_mutex);
  const auto result=_serial_connection->send_message(message.m);
  if(!result){
    // cannot send - probably disconnected
  }

}
void XMavsdkWrapperSerialConnection::constantConnect() {
  while (true){
    const auto result=_serial_connection->start();
    if(result==mavsdk::ConnectionResult::Success){
      return;
    }
    std::cout<<"XMavsdkWrapperSerialConnection:"<<result<<"\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
}