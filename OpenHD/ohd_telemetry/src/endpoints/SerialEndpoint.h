//
// Created by consti10 on 13.04.22.
//

#ifndef XMAVLINKSERVICE_SERIALENDPOINT_H
#define XMAVLINKSERVICE_SERIALENDPOINT_H

#include <utility>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <chrono>

#include "MEndpoint.hpp"

// UART endpoint
// Supports sending (mavlink) messages to the connected UART device (aka the flight controller)
// and receiving (mavlink) messages from it

// Should do's
// 1) Handle a disconnecting flight controller: When the flight controller uart for some reason is disconnected,
// try to reconnect
// 2) Handle no flight controller at all: Simple, just do nothing until a flight controller is connected

// TODO: Maybe replace all the boost stuff with https://github.com/mavlink/c_uart_interface_example/blob/master/serial_port.cpp
class SerialEndpoint : public MEndpoint {
 public:
  struct HWOptions {
	std::string linux_filename; // the linux file name,for example /dev/tty..
	int baud_rate = 0; // manual baud rate, set to 0 to leave untouched
  };
 public:
  /**
   * @param serial_port the serial port linux name (dev/.. ) for this serial port
   */
  explicit SerialEndpoint(std::string TAG, HWOptions options,bool enableDebug=false);
  //
  static constexpr auto USB_SERIAL_PORT = "/dev/ttyUSB0";
  static constexpr auto TEST_SERIAL_PORT = "/dev/ttyACM0";
 private:
  void sendMessageImpl(const MavlinkMessage &message) override;
  // If the serial port is still opened, close it
  // after that, it should be openable again
  void safeCloseCleanup();
  // loops until the serial port has been opened successfully, then calls read
  void safeRestart();
  // Async receive some data, when done (and no error occurred) this is called asynchronous again.
  void startReceive();
  void handleRead(const boost::system::error_code &error,
				  size_t bytes_transferred);
  void handleWrite(const boost::system::error_code &error,
				   size_t bytes_transferred);
  const HWOptions m_options;
  boost::asio::io_service io_service;
  boost::asio::serial_port m_serial;
  std::array<uint8_t, 1024> readBuffer{};
  static constexpr auto RECONNECT_DELAY = std::chrono::seconds(5);
 private:
  std::unique_ptr<boost::thread> mIoThread;
  std::unique_ptr<boost::thread> mOpenSerialPortThread;
  const bool m_enable_debug;
};

#endif //XMAVLINKSERVICE_SERIALENDPOINT_H
