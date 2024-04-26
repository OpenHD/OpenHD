//
// Created by consti10 on 31.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_

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
 *  At some point, I decided there is no way around it than to write our own
 * UART receiver program. Shares some similarities with MAVSDK's serial endpoint
 * implementation. It is safe to create an instance of this class with common
 * mistakes like a wrong serial fd - In this case, this will constantly log some
 * "warning messages" until the issue is fixed (for example by the user
 * connecting the serial wires, or selecting another type of fd)
 */
class SerialEndpoint : public MEndpoint {
 public:
  struct HWOptions {
    std::string linux_filename;  // the linux file name,for example /dev/tty..
    int baud_rate = 115200;      // manual baud rate
    bool flow_control = false;   // pretty much never used
    // constantly read data from the connected device. Can be disabled in case
    // the connected device shall only receive data (e.g. a tracker on the
    // ground)
    bool enable_reading = true;
    bool enable_debug = false;  // enable / disable extra debug logging
    [[nodiscard]] std::string to_string() const {
      std::stringstream ss;
      ss << "HWOptions{" << linux_filename << ", baud:" << baud_rate
         << ", flow_control:" << flow_control
         << ",  enable_debug:" << enable_debug << "}";
      return ss.str();
    }
  };

 public:
  /**
   * See @param options1 HWOptions for configurable serial params
   */
  explicit SerialEndpoint(std::string TAG1, HWOptions options1);
  // No copy and move
  SerialEndpoint(const SerialEndpoint&) = delete;
  SerialEndpoint(const SerialEndpoint&&) = delete;
  ~SerialEndpoint();
  // Start sending and receiving UART data.
  // Does nothing if already started.
  void start();
  // Stop any UART communication (read and write). Might block for up to 1
  // second. Does nothing if already stopped.
  void stop();
  // Linux defines what baud rates are available - this does not check if the
  // given baud rate is actually supported by the HW, but checks if it is at
  // least a somewhat sane value
  static bool is_valid_linux_baudrate(int baudrate);

 private:
  bool sendMessagesImpl(const std::vector<MavlinkMessage>& messages) override;
  static int define_from_baudrate(int baudrate);
  static int setup_port(const HWOptions& options,
                        std::shared_ptr<spdlog::logger> m_console);
  void connect_and_read_loop();
  // Receive data until either an error occurs (in this case, the UART most
  // likely disconnected) Or a stop was requested.
  void receive_data_until_error();
  // Write serial data, returns true on success, false otherwise.
  [[nodiscard]] bool write_data_serial(const std::vector<uint8_t>& data);

 private:
  const HWOptions m_options;
  int m_fd = -1;
  std::mutex m_connect_receive_thread_mutex;
  std::unique_ptr<std::thread> m_connect_receive_thread = nullptr;
  bool _stop_requested = false;
  std::shared_ptr<spdlog::logger> m_console;
  // Limit warning console logs to not spam the console
  static constexpr auto MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES =
      std::chrono::seconds(3);
  std::chrono::steady_clock::time_point m_last_log_serial_write_failed =
      std::chrono::steady_clock::now();
  int m_n_failed_writes = 0;
  // Limit warning console logs to not spam the console
  static constexpr auto MIN_DELAY_BETWEEN_SERIAL_READ_FAILED_LOG_MESSAGES =
      std::chrono::seconds(3);
  std::chrono::steady_clock::time_point m_last_log_serial_read_failed =
      std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point m_last_log_cannot_send_no_fd =
      std::chrono::steady_clock::now();
  int m_n_failed_reads = 0;
  // std::unique_ptr<openhd::log::LimitedRateLogger> m_limited_rate_logger;
};

// OpenHD supports enabling / disabling and changing the Serial at run time, but
// SerialEndpoint itself is not mutable
// - this class exists to abstract away common multi threading issues in OpenHD
// that arise from this need
class SerialEndpointManager {
 public:
  /**
   * Send messages if serial is currently enabled, otherwise, do nothing
   */
  void send_messages_if_enabled(const std::vector<MavlinkMessage>& messages);
  /**
   * (Re-)configure the wrapped serial endpoint. Stops then restarts if serial
   * already exists
   * @param cb the cb that is called regularly with new messages if enabled
   */
  void configure(const SerialEndpoint::HWOptions& options,
                 const std::string& tag, MAV_MSG_CALLBACK cb);
  /**
   * Disable (delete) serial, if already existing
   */
  void disable();

 private:
  std::unique_ptr<SerialEndpoint> m_serial_endpoint;
  std::mutex m_serial_endpoint_mutex;
  std::shared_ptr<spdlog::logger> m_console =
      openhd::log::create_or_get("ser_manager");
};

// Serial is a string param, where empty and some name(s) have special meaning
// (They map to specific serial FD name)
// Here we return std::null-opt if serial is disabled ("")
// otherwise, a (most likely) valid serial fd path
std::optional<std::string> serial_openhd_param_to_linux_fd(
    const std::string& param_name);

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_ENDPOINTS_SERIALENDPOINT_H_
