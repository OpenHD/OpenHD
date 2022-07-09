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

  const auto result=_serial_connection->start();
  std::cout<<"XMavsdkWrapperSerialConnection:"<<result;
}

void XMavsdkWrapperSerialConnection::sendMessageImpl(
    const MavlinkMessage& message) {
  const auto result=_serial_connection->send_message(message.m);
  if(!result){
    // cannot send - probably disconnected
  }

}

}