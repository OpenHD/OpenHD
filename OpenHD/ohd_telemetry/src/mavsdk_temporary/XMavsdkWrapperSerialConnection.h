//
// Created by consti10 on 09.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_

#include "../endpoints/MEndpoint.hpp"
#include "serial_connection.h"
#include <memory>
#include <thread>
#include <optional>

namespace mavsdk{

class XMavsdkWrapperSerialConnection : public MEndpoint{
 public:
  explicit XMavsdkWrapperSerialConnection(
      const std::optional<std::string> path,
      int baudrate,
      bool flow_control=false);
 private:
  void sendMessageImpl(const MavlinkMessage &message) override;
  std::optional<std::string> check_serial_file_handles();
 private:
  [[noreturn]] void constantConnect();
  Connection::receiver_callback_t _for_mavsdk_receiver_callback;
  std::unique_ptr<SerialConnection> _serial_connection;
  std::unique_ptr<std::thread> establish_connection_thread = nullptr;
  std::mutex _mutex{};
  std::optional<std::string> _path;
  int _baudrate;
  bool _flow_control;
};

}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAVSDK_PARAM_XMAVSDKWRAPPERSERIALCONNECTION_H_
