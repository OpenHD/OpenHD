//
// Created by consti10 on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "MEndpoint.h"
#include "openhd_spdlog.h"

// At some point, I decided there is no way around it than to write our own UART receiver program.
// Mostly based on MAVSDK. Doesn't use boost.
class SerialEndpoint : public MEndpoint{
 public:
  struct HWOptions {
    std::string linux_filename; // the linux file name,for example /dev/tty..
    int baud_rate = 115200; // manual baud rate
    bool flow_control=false; // pretty much never used
    // constantly read data from the connected device. Can be disabled in case the connected device shall only receive data
    // (e.g. a tracker on the ground)
    bool enable_reading= true;
    bool enable_debug=false; // enable / disable extra debug logging
    [[nodiscard]] std::string to_string() const{
      std::stringstream ss;
      ss<<"HWOptions{"<<linux_filename<<", baud:"<<baud_rate<<", flow_control:"<<flow_control<<",  enable_debug:"<<enable_debug<<"}";
      return ss.str();
    }
  };
 public:
  /**
   * See @param options1 HWOptions for configurable serial params
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
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  static int define_from_baudrate(int baudrate);
  static int setup_port(const HWOptions& options,std::shared_ptr<spdlog::logger> m_console);
  void connect_and_read_loop();
  // Receive data until either an error occurs (in this case, the UART most likely disconnected)
  // Or a stop was requested.
  void receive_data_until_error();
  // Write serial data, returns true on success, false otherwise.
  [[nodiscard]] bool write_data_serial(const std::vector<uint8_t>& data);
 private:
  const HWOptions m_options;
  int m_fd =-1;
  std::mutex m_connect_receive_thread_mutex;
  std::unique_ptr<std::thread> m_connect_receive_thread = nullptr;
  bool _stop_requested=false;
  std::shared_ptr<spdlog::logger> m_console;
  // Limit warning console logs to not spam the console
  static constexpr auto MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES=std::chrono::seconds(3);
  std::chrono::steady_clock::time_point m_last_log_serial_write_failed=std::chrono::steady_clock::now();
  int m_n_failed_writes =0;
  // Limit warning console logs to not spam the console
  static constexpr auto MIN_DELAY_BETWEEN_SERIAL_READ_FAILED_LOG_MESSAGES=std::chrono::seconds(3);
  std::chrono::steady_clock::time_point m_last_log_serial_read_failed=std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point m_last_log_cannot_send_no_fd=std::chrono::steady_clock::now();
  int m_n_failed_reads=0;
  //std::unique_ptr<openhd::log::LimitedRateLogger> m_limited_rate_logger;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
