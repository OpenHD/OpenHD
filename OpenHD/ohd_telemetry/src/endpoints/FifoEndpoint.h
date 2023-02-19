//
// Created by rsaxvc on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_FIFOENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_FIFOENDPOINT_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "MEndpoint.h"
#include "openhd_spdlog.h"

/**
 *  MAVLink endpoint for a pair of Unix FIFOs acting as a virtual serial port..
 *  Shares many similarities with OpenHD's SerialEndpoint implementation.
 *  It is safe to create an instance of this class with common mistakes like a wrong FIFO filename -
 *  In this case, this will constantly log some "warning messages" until the issue is fixed.
 */
class FifoEndpoint : public MEndpoint{
 public:
  struct HWOptions {
    // the linux file name,for example /dev/tty..
    // except in the case of FIFO, rx should match tx.
    std::string linux_filename_rx="/run/openhd/ohdFifoRx";
    std::string linux_filename_tx="/run/openhd/ohdFifoTx";
    // constantly read data from the connected device. Can be disabled in case the connected device shall only receive data
    // (e.g. a tracker on the ground)
    bool enable_reading= true;
    bool enable_debug=false; // enable / disable extra debug logging
    [[nodiscard]] std::string to_string() const{
      std::stringstream ss;
      ss<<"HWOptions{rx:"<<linux_filename_rx<<", tx:"<<linux_filename_tx<<",  enable_debug:"<<enable_debug<<"}";
      return ss.str();
    }
  };
 public:
  /**
   * See @param options1 HWOptions for configurable params
   */
  explicit FifoEndpoint(std::string TAG1, HWOptions options1);
  // No copy and move
  FifoEndpoint(const FifoEndpoint&)=delete;
  FifoEndpoint(const FifoEndpoint&&)=delete;
  ~FifoEndpoint();
  // Start sending and receiving FIFO data.
  // Does nothing if already started.
  void start();
  // Stop any FIFO communication (read and write). Might block for up to 1 second.
  // Does nothing if already stopped.
  void stop();
 private:
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  static bool isafifo(int fd);
  static int setup_fifo_rx(const HWOptions& options,std::shared_ptr<spdlog::logger> m_console);
  static int setup_fifo_tx(const HWOptions& options,std::shared_ptr<spdlog::logger> m_console);
  void connect_and_read_loop();
  // Receive data until either an error occurs (in this case, the FIFOs most likely disconnected)
  // Or a stop was requested.
  void receive_data_until_error();
  // Write FIFO data, returns true on success, false otherwise.
  [[nodiscard]] bool write_data_fifo(const std::vector<uint8_t>& data);
 private:
  const HWOptions m_options;
  int m_fd_rx=-1;
  int m_fd_tx=-1;
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

// OpenHD supports enabling / disabling and changing the Serial at run time, but FifoEndpoint itself is not mutable
// - this class exists to abstract away common multi threading issues in OpenHD that arise from this need
class FifoEndpointManager{
 public:
  /**
   * Send messages if serial is currently enabled, otherwise, do nothing
   */
  void send_messages_if_enabled(const std::vector<MavlinkMessage>& messages);
  /**
   * (Re-)configure the wrapped serial endpoint. Stops then restarts if serial already exists
   * @param cb the cb that is called regularly with new messages if enabled
   */
  void configure(const FifoEndpoint::HWOptions& options,const std::string& tag,MAV_MSG_CALLBACK cb);
  /**
   * Disable (delete) serial, if already existing
   */
  void disable();
 private:
  std::unique_ptr<FifoEndpoint> m_serial_endpoint;
  std::mutex m_serial_endpoint_mutex;
  std::shared_ptr<spdlog::logger> m_console=openhd::log::create_or_get("fifo_manager");
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_FIFOENDPOINT_H_
