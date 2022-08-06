//
// Created by consti10 on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_

#include "MEndpoint.hpp"
#include <utility>
#include <chrono>
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

// At some point, I decided there is no way around it than to write our own UART receiver program.
// Mostly based on MAVSDK. Doesn't use boost.
class SerialEndpoint : public MEndpoint{
 public:
  struct HWOptions {
	std::string linux_filename; // the linux file name,for example /dev/tty..
	int baud_rate = 115200; // manual baud rate
	bool flow_control=false; // not tested yet
	bool enable_debug=false; //
	[[nodiscard]] std::string to_string() const{
	  std::stringstream ss;
	  ss<<"HWOptions{"<<linux_filename<<", baud:"<<baud_rate<<", flow_control:"<<flow_control<<",  enable_debug:"<<enable_debug<<"}";
	  return ss.str();
	}
  };
 public:
  /**
   * @param serial_port the serial port linux name (dev/.. ) for this serial port
   */
  explicit SerialEndpoint(std::string TAG1, HWOptions options1);
  // No copy and move
  SerialEndpoint(const SerialEndpoint&)=delete;
  SerialEndpoint(const SerialEndpoint&&)=delete;
  ~SerialEndpoint();
  // Start sending and receiving UART data.
  // Does nothing if already started.
  void start();
  // Stop any UART communication (read and write). Might block for up to 1 second.
  // Does nothing if already stopped.
  void stop();
 private:
  bool sendMessageImpl(const MavlinkMessage &message) override;
  static int define_from_baudrate(int baudrate);
  static int setup_port(const HWOptions& options);
  void connect_and_read_loop();
  // Receive data until either an error occurs (in this case, the UART most likely disconnected)
  // Or a stop was requested.
  void receive_data_until_error();
  // Write serial data, returns true on success, false otherwise.
  [[nodiscard]] bool write_data_serial(const std::vector<uint8_t>& data) const;
 private:
  const HWOptions _options;
  int _fd=-1;
  std::mutex _connectReceiveThreadMutex;
  std::unique_ptr<std::thread> _connectReceiveThread = nullptr;
  bool _stop_requested=false;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
