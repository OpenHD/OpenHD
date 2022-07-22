//
// Created by consti10 on 09.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_

#include "../endpoints/MEndpoint.hpp"
#include "serial_connection.h"
#include <memory>
#include <thread>

namespace mavsdk{

class XMavsdkWrapperSerialConnection : public MEndpoint{
 public:
  explicit XMavsdkWrapperSerialConnection(
      const std::string& path,
      int baudrate,
      bool flow_control=false);
 private:
  void sendMessageImpl(const MavlinkMessage &message) override;
 private:
  void constantConnect();
  Connection::receiver_callback_t _for_mavsdk_receiver_callback;
  std::unique_ptr<SerialConnection> _serial_connection;
  std::unique_ptr<std::thread> establish_connection_thread = nullptr;
  std::mutex _mutex{};
  const std::string _path;
  bool _started=false;
};

}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_
